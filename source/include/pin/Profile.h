#ifndef HEAAN_PROFILE_H_
#define HEAAN_PROFILE_H_

#define MAX_PROFILE_FUNC 1000
extern double timeResult[MAX_PROFILE_FUNC];

#define NEGATIVE(x) ((-1)*x)

#define BASE_NORMAL_ADD 100
#define BASE_HE_ADD 200
#define BASE_NORMAL_MULT 300
#define BASE_HE_MULT 400
#define BASE_BOOT_STRAP 500


/*

#include "TimeUtils.h"
#include "Profile.h"

        initTimeResult();

#ifdef _TIME_PERFORM_
        TimeUtils timeutils;
        timeutils.startMid(__func__);
#endif
#ifdef _PROFILE_
        startProfile();
#endif
#ifdef _PROFILE_
        endProfile(NEGATIVE());
#endif
#ifdef _TIME_PERFORM_
        timeResult[] += timeutils.stopMid(__func__);
#endif


#ifdef _TIME_PERFORM_
        TimeUtils timeutils;
        timeutils.startMid(__func__);
#endif

#ifdef _BS_PROFILE_
        startProfile();
#endif
#ifdef _BS_PROFILE_
        endProfile(NEGATIVE());
#endif
#ifdef _TIME_PERFORM_
        timeResult[] += timeutils.stopMid(__func__);
#endif

*/

enum _PROFILE_FUNC_ {
    NORMAL_ADD = BASE_NORMAL_ADD,

    HE_ADD = BASE_HE_ADD,
    RING_addAndEqual,

    NORMAL_MULT = BASE_NORMAL_MULT,

    HE_MULT = BASE_HE_MULT,
    RING_CRT,
    RING_multDNTT,
    RING_addNTTAndEqual,
    RING_rightShiftAndEqual,
    RING_subAndEqual,
    RING_leftRotate,
    RING_MULT_CRT,
    RING_MULT_NTT,
    RING_MULT_multDNTT,
    RING_MULT_INTT,
    RING_MULT_reconstruct,
    RING_MULT_addNTTAndEqual,
    RING_MULT_multNTT,
    RING_MULT_squareNTT,

    BOOT_STRAP = BASE_BOOT_STRAP,
    BOOT_STRAP_SUB_SUM,
    BOOT_STRAP_CoeffToSlot,
    BOOT_STRAP_EvalExp,
    BOOT_STRAP_SlotToCoeff,
    END_PROFILE_FUNC
};


int startProfile(int ctl);
int endProfile(int ctl);


#endif
