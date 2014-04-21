#include "NoiseFilterer.h"

#include <assert.h>


#pragma region TwoD Noise

typedef NoiseFilterer2D NF2;

const float sqrt2 = sqrt(2.0f),
	        sqrt2Inv = 1.0f / sqrt2;

void NF2::ReflectValues(Noise2D * nse) const
{
    if (nse != 0) noise = nse;

	struct RefValuesStruct { Interval i; };
	RefValuesStruct rvs;
	rvs.i = Interval::GetZeroToOneInterval();

	SetAtEveryPoint((void*)&rvs, [](void *pData, Vector2i loc, Noise2D * nse) { return (*(RefValuesStruct*)pData).i.Reflect((*nse)[loc]); });
}

void NF2::RemapValues(Noise2D * nse) const
{
    if (nse != 0) noise = nse;

	Interval newVals = Interval::GetZeroToOneInterval();
	Interval oldVals = RemapValues_OldVals;

	struct RemapValuesStruct { Interval oldVs, newVs; Noise2D * nse; };
	RemapValuesStruct rvs;
	rvs.oldVs = oldVals;
	rvs.newVs = newVals;
	rvs.nse = noise;

	FillRegion->DoToEveryPoint((void*)&rvs, [](void * pDat, Vector2i loc, float strength)
	{
		RemapValuesStruct * rvS = (RemapValuesStruct*)pDat;
		float * fOut = &(*(rvS->nse))[loc];
		*fOut = rvS->newVs.Clamp(rvS->oldVs.MapValue(rvS->newVs, *fOut));
	},
	*noise, Vector2i(noise->GetWidth(), noise->GetHeight()));
}

void NF2::UpContrast(Noise2D * nse) const
{
    if (nse != 0) noise = nse;

    //Struct to pass into the lambda.
    struct UpContrastArgs
    {
    public:
        float(*SmoothStepper)(float inF);
        unsigned int Iterations;
    };

    UpContrastArgs args;
    args.Iterations = UpContrast_Passes;
	switch (UpContrast_Power)
	{
		case UpContrastPowers::CUBIC:
			args.SmoothStepper = &BasicMath::Smooth;
			break;
		case UpContrastPowers::QUINTIC:
			args.SmoothStepper = &BasicMath::Supersmooth;
			break;

		default: assert(false);
	}
    

	SetAtEveryPoint((void*)(&args), [](void * pDat, Vector2i loc, Noise2D * noise)
	{
        UpContrastArgs rgs = *(UpContrastArgs*)pDat;
        float val = (*noise)[loc];

        for (unsigned int pass = 0; pass < rgs.Iterations; ++pass)
            val = rgs.SmoothStepper(val);

        return val;
	});
}

void NF2::Average(Noise2D * nse) const
{
    if (nse != 0) noise = nse;

	struct AverageStruct { float average; int count; Noise2D * nse; AverageStruct(void) : average(0.0f), count(0) { } };
	AverageStruct avs;
	avs.nse = noise;

	FillRegion->DoToEveryPoint((void*)&avs, [](void* pData, Vector2i loc, float str)
	{
		AverageStruct * avS = (AverageStruct*)pData;

		avS->average += (*(avS->nse))[loc];
		avS->count += 1;
	}, *noise, Vector2i(noise->GetWidth(), noise->GetHeight()), false);

	if (avs.count > 0)
	{
		avs.average /= (float)avs.count;
	}

	SetAtEveryPoint((void*)&avs.average, [](void * pData, Vector2i loc, Noise2D * noise) { return *(float*)pData; });
}

void NF2::Flatten(Noise2D * nse) const
{
    if (nse != 0) noise = nse;

	SetAtEveryPoint((void*)&Flatten_FlatValue, [](void *pData, Vector2i loc, Noise2D * noise) { return *(float*)pData; });
}

void NF2::Smooth(Noise2D * nse) const
{
    if (nse != 0) noise = nse;

	SetAtEveryPoint((void*)0, [](void *pData, Vector2i loc, Noise2D * noise)
	{
		int x, y, lX, lY;

		float average = 0.0f;
		int count = 0;
		Vector2i locTemp;

		//Average the surrounding values.
		for (x = -1; x <= 1; ++x)
		{
			lX = x + loc.x;
			locTemp.x = lX;

			if (lX >= 0 && lX < noise->GetWidth())
			{
				for (y = -1; y <= 1; ++y)
				{
					lY = y + loc.y;
					locTemp.y = lY;

					if ((x != 0 || y != 0) &&
						lY >= 0 && lY < noise->GetHeight())
					{
						average += (*noise)[locTemp];
						count += 1;
					}
				}
			}
		}

		if (count > 0)
		{
			average /= (float)count;
		}

		return average;
	});
}

void NF2::Increase(Noise2D * nse) const
{
    if (nse != 0) noise = nse;

	SetAtEveryPoint((void*)&Increase_Amount, [](void *pData, Vector2i loc, Noise2D * noise)
	{
		return (*noise)[loc] + *(float*)pData;
	});
}

void NF2::Min(Noise2D * nse) const
{
    if (nse != 0) noise = nse;

    SetAtEveryPoint((void*)&Min_Value, [](void *pData, Vector2i loc, Noise2D * noise)
    {
        return BasicMath::Min((*noise)[loc], *(float*)pData);
    });
}
void NF2::Max(Noise2D * nse) const
{
    if (nse != 0) noise = nse;

    SetAtEveryPoint((void*)&Max_Value, [](void *pData, Vector2i loc, Noise2D * noise)
    {
        return BasicMath::Max((*noise)[loc], *(float*)pData);
    });
}
void NF2::Clamp(Noise2D * nse) const
{
    if (nse != 0) noise = nse;

    float fs[2] = { Min_Value, Max_Value };
    SetAtEveryPoint((void*)fs, [](void *pData, Vector2i loc, Noise2D * noise)
    {
        return BasicMath::Clamp((*noise)[loc], ((float*)pData)[0], ((float*)pData)[1]);
    });
}

void NF2::Noise(Noise2D * nse) const
{
    if (nse != 0) noise = nse;

	struct NoiseStruct { float amount; int seed; FastRand fr; };
	NoiseStruct ns;
	ns.amount = Noise_Amount;
	ns.seed = Noise_Seed;
	ns.fr = FastRand(Noise_Seed);

	SetAtEveryPoint((void*)&ns, [](void *pData, Vector2i loc, Noise2D * noise)
	{
		NoiseStruct * nS = (NoiseStruct*)pData;

		nS->fr.Seed = Vector3i(loc.x, loc.y, nS->seed).GetHashCode();
		return (*noise)[loc] + (nS->amount * nS->fr.GetZeroToOne());
	});
}

void NF2::Generate(Noise2D & dat) const
{
	if (NoiseToFilter == 0 || FillRegion == 0 || FilterFunc == 0) return;

	NoiseToFilter->Generate(dat);

	noise = &dat;
	((*this).*FilterFunc)(0);
}

void NF2::SetAtEveryPoint(void * pData, float (*GetValue)(void * pData, Vector2i loc, Noise2D * noise)) const
{
	//Package necessary information into a struct.
	struct SAEPData { void * PData; bool Invert; Noise2D * Noise; float(*PGetValue)(void * pData, Vector2i loc, Noise2D * noise); };
	SAEPData d;
	d.PData = pData;
	d.PGetValue = GetValue;
	d.Noise = noise;
	d.Invert = InvertFunc;

	//Fill the noise.
	FillRegion->DoToEveryPoint((void*)(&d), [](void *pData, Vector2i loc, float strength)
	{
		//Unpack the information.
		SAEPData * dt = (SAEPData*)pData;
		void *PD = dt->PData;
		float (*pGV)(void * pData, Vector2i loc, Noise2D * pNoise) = dt->PGetValue;

		//Get the original and new value.
		float preVal = pGV(PD, loc, dt->Noise),
			  orgVal = dt->Noise->operator[](loc);

		//If necessary, invert the effect of the filter function.
		if (dt->Invert)
		{
			preVal = orgVal - (preVal - orgVal);
		}

		//Lerp between the old and new value using the strength value.
		dt->Noise->operator[](loc) = BasicMath::Clamp(BasicMath::Lerp(orgVal, preVal, strength), 0.0f, 1.0f);
	},
	*noise, Vector2i(noise->GetWidth(), noise->GetHeight()));
}

#pragma endregion



#pragma region ThreeD Noise

typedef NoiseFilterer3D NF3;


void NF3::ReflectValues(Noise3D * nse) const
{
    struct RefValuesStruct { Interval i; };
    RefValuesStruct rvs;
    rvs.i = Interval::GetZeroToOneInterval();

    SetAtEveryPoint(nse, (void*)&rvs, [](void *pData, Vector3i loc, Noise3D * nse) { return (*(RefValuesStruct*)pData).i.Reflect((*nse)[loc]); });
}

void NF3::RemapValues(Noise3D * nse) const
{
    Interval newVals = Interval::GetZeroToOneInterval();
    Interval oldVals = RemapValues_OldVals;

    struct RemapValuesStruct { Interval oldVs, newVs; Noise3D * nse; };
    RemapValuesStruct rvs;
    rvs.oldVs = oldVals;
    rvs.newVs = newVals;
    rvs.nse = nse;

    FillVolume->DoToEveryPoint((void*)&rvs, [](void * pDat, Vector3i loc, float strength)
    {
        RemapValuesStruct * rvS = (RemapValuesStruct*)pDat;
        float * fOut = &(*(rvS->nse))[loc];
        *fOut = rvS->newVs.Clamp(rvS->oldVs.MapValue(rvS->newVs, *fOut));
    },
        *nse, Vector3i(nse->GetWidth(), nse->GetHeight(), nse->GetDepth()));
}

void NF3::UpContrast(Noise3D * nse) const
{
    //Struct to pass into the lambda.
    struct UpContrastArgs
    {
    public:
        float(*SmoothStepper)(float inF);
        unsigned int Iterations;
    };

    UpContrastArgs args;
    args.Iterations = UpContrast_Passes;
    switch (UpContrast_Power)
    {
    case UpContrastPowers::CUBIC:
        args.SmoothStepper = &BasicMath::Smooth;
        break;
    case UpContrastPowers::QUINTIC:
        args.SmoothStepper = &BasicMath::Supersmooth;
        break;

    default: assert(false);
    }


    SetAtEveryPoint(nse, (void*)(&args), [](void * pDat, Vector3i loc, Noise3D * noise)
    {
        UpContrastArgs rgs = *(UpContrastArgs*)pDat;
        float val = (*noise)[loc];

        for (unsigned int pass = 0; pass < rgs.Iterations; ++pass)
            val = rgs.SmoothStepper(val);

        return val;
    });
}

void NF3::Average(Noise3D * nse) const
{
    struct AverageStruct { float average; int count; Noise3D * nse; AverageStruct(void) : average(0.0f), count(0) { } };
    AverageStruct avs;
    avs.nse = nse;

    FillVolume->DoToEveryPoint((void*)&avs, [](void* pData, Vector3i loc, float str)
    {
        AverageStruct * avS = (AverageStruct*)pData;

        avS->average += (*(avS->nse))[loc];
        avS->count += 1;
    }, *nse, Vector3i(nse->GetWidth(), nse->GetHeight(), nse->GetDepth()), false);

    if (avs.count > 0)
    {
        avs.average /= (float)avs.count;
    }

    SetAtEveryPoint(nse, (void*)&avs.average, [](void * pData, Vector3i loc, Noise3D * noise) { return *(float*)pData; });
}

void NF3::Set(Noise3D * nse) const
{
    SetAtEveryPoint(nse, (void*)&Set_Value, [](void *pData, Vector3i loc, Noise3D * noise) { return *(float*)pData; });
}

void NF3::Smooth(Noise3D * nse) const
{
    SetAtEveryPoint(nse, (void*)0, [](void *pData, Vector3i lc, Noise3D * noise)
    {
        //Average the surrounding values.
        Vector3i noiseLoc;
        float avg = 0.0f;
        int count = 0;
        for (Vector3i loc(0, 0, -1); loc.z <= 1; ++loc.z)
        {
            noiseLoc.z = loc.z + lc.z;

            if (noiseLoc.z < 0 || noiseLoc.z >= noise->GetDepth())
                continue;

            for (loc.y = -1; loc.y <= 1; ++loc.y)
            {
                noiseLoc.y = loc.y + lc.y;

                if (noiseLoc.y < 0 || noiseLoc.y >= noise->GetHeight())
                    continue;

                for (loc.x = -1; loc.x <= 1; ++loc.x)
                {
                    noiseLoc.x = loc.x + lc.x;

                    //Only count the value if it isn't the center value.
                    if ((loc.x != 0 || loc.y != 0 || loc.z != 0) &&
                        (noiseLoc.x >= 0 && noiseLoc.x < noise->GetWidth()))
                    {
                        avg += (*noise)[noiseLoc];
                        count += 1;
                    }
                }
            }
        }

        if (count > 0)
            avg /= (float)count;
        else avg = 0.5f;

        return avg;
    });
}

void NF3::Increase(Noise3D * nse) const
{
    SetAtEveryPoint(nse, (void*)&Increase_Amount, [](void *pData, Vector3i loc, Noise3D * noise)
    {
        return (*noise)[loc] + *(float*)pData;
    });
}

void NF3::Min(Noise3D * nse) const
{
    SetAtEveryPoint(nse, (void*)&Min_Value, [](void *pData, Vector3i loc, Noise3D * noise)
    {
        return BasicMath::Min((*noise)[loc], *(float*)pData);
    });
}
void NF3::Max(Noise3D * nse) const
{
    SetAtEveryPoint(nse, (void*)&Max_Value, [](void *pData, Vector3i loc, Noise3D * noise)
    {
        return BasicMath::Max((*noise)[loc], *(float*)pData);
    });
}
void NF3::Clamp(Noise3D * nse) const
{
    float fs[2] = { Min_Value, Max_Value };
    SetAtEveryPoint(nse, (void*)fs, [](void *pData, Vector3i loc, Noise3D * noise)
    {
        return BasicMath::Clamp((*noise)[loc], ((float*)pData)[0], ((float*)pData)[1]);
    });
}

void NF3::Noise(Noise3D * nse) const
{
    struct NoiseStruct { float amount; int seed; FastRand fr; };
    NoiseStruct ns;
    ns.amount = Noise_Amount;
    ns.seed = Noise_Seed;
    ns.fr = FastRand(Noise_Seed);

    SetAtEveryPoint(nse, (void*)&ns, [](void *pData, Vector3i loc, Noise3D * noise)
    {
        NoiseStruct * nS = (NoiseStruct*)pData;

        nS->fr.Seed = Vector4i(loc.x, loc.y, loc.z, nS->seed).GetHashCode();

        return (*noise)[loc] + (nS->amount * (-1.0f + (2.0f * nS->fr.GetZeroToOne())));
    });
}

void NF3::SetAtEveryPoint(Noise3D * nse, void * pData, float(*GetValue)(void * pData, Vector3i loc, Noise3D * noise)) const
{
    //Package necessary information into a struct.
    struct SAEPData { void * PData; bool Invert; Noise3D * Noise; float(*PGetValue)(void * pData, Vector3i loc, Noise3D * noise); };
    SAEPData d;
    d.PData = pData;
    d.PGetValue = GetValue;
    d.Noise = nse;
    d.Invert = InvertFunc;

    //Fill the noise.
    FillVolume->DoToEveryPoint((void*)(&d), [](void *pData, Vector3i loc, float strength)
    {
        //Unpack the information.
        SAEPData * dt = (SAEPData*)pData;
        void *PD = dt->PData;
        float(*pGV)(void * pData, Vector3i loc, Noise3D * pNoise) = dt->PGetValue;

        //Get the original and new value.
        float preVal = pGV(PD, loc, dt->Noise),
              orgVal = dt->Noise->operator[](loc);

        //If necessary, invert the effect of the filter function.
        if (dt->Invert)
        {
            preVal = orgVal - (preVal - orgVal);
        }

        //Lerp between the old and new value using the strength value.
        dt->Noise->operator[](loc) = BasicMath::Clamp(BasicMath::Lerp(orgVal, preVal, strength), 0.0f, 1.0f);
    }, *nse, Vector3i(nse->GetWidth(), nse->GetHeight(), nse->GetDepth()));
}

#pragma endregion