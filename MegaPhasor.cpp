// 2**24
#define BUFLEN 16777216

#include "SC_PlugIn.hpp"

// InterfaceTable contains pointers to functions in the host (server).
static InterfaceTable *ft;

// declare struct to hold unit generator state
struct MegaPhasor : public SCUnit{

// Constructor usually does 3 things.
// 1. set the calculation function.
// 2. initialize the unit generator state variables.
// 3. calculate one sample of output.
public:
    MegaPhasor() {
        // 1. set the calculation function.
        if (isAudioRateIn(2)) {
            if (isAudioRateIn(3)) {
                // both trig and rate are audio rate
                set_calc_function<MegaPhasor,&MegaPhasor::next_aa>();
            } else {
                // only trig is audio rate
                set_calc_function<MegaPhasor,&MegaPhasor::next_ak>();
            }

        } else {
            if (isAudioRateIn(3)) {
                // only rate is audio rate
                set_calc_function<MegaPhasor,&MegaPhasor::next_ka>();
            } else {
                // nothing is audio rate
                set_calc_function<MegaPhasor,&MegaPhasor::next_kk>();
            }
        }

        // 2. initialize the unit generator state variables.
        mNumbufs = (int)in0(0);
        mOverlap = (int)in0(1);
        mPrevtrig = in0(2);
        mBuf = (int)in0(4);
        mPos = in0(5);
        mBeginbuf = (int)in0(6);
        mBeginpos = in0(7);
        mEndbuf = (int)in0(8);
        mEndpos = in0(9);
        mPlaying = 0;

        // 3. calculate one sample of output.
        next_kk(1);
    }

private:
    int mNumbufs;
    int mOverlap;
    float mPrevtrig;
    int mBuf;
    float mPos;
    int mBeginbuf;
    float mBeginpos;
    int mEndbuf;
    float mEndpos;
    int mPlaying;



    //////////////////////////////////////////////////////////////////

    // The calculation function executes once per control period
    // which is typically 64 samples.

    // calculation function for all control rate arguments
    void next_kk(int inNumSamples)
    {
        // get the pointer to the output buffer
        float *outBuf0 = out(0);
        float *outPos0 = out(1);
        float *outBuf1 = out(2);
        float *outPos1 = out(3);
        float *outXfade = out(4);
        float *outPlaying = out(5);

        const float trig = in0(2);
        const float rate = in0(3);
        const int startbuf = in0(4);
        const float startpos = in0(5);
        const int loop = (int)in0(10);

        // get data from struct and store it in a
        // local variable.
        // The optimizer will cause them to be loaded it into a register.
        int numbufs = mNumbufs;
        int overlap = mOverlap;
        float prevtrig = mPrevtrig;
        int buf = mBuf;
        float pos = mPos;
        int beginbuf = mBeginbuf;
        float beginpos = mBeginpos;
        int endbuf = mEndbuf;
        float endpos = mEndpos;
        int playing = mPlaying;

        int bufspacing = BUFLEN - overlap;

        if (prevtrig <= 0.f && trig > 0.f) {
            buf = startbuf;
            pos = startpos;
            playing = 0;
        }

        int player;

        // perform a loop for the number of samples in the control period.
        for (int i=0; i < inNumSamples; ++i)
        {
            player = buf % 2;

            // write the output
            if (player == 0) {
                outBuf0[i] = buf;
                outPos0[i] = pos;
                if (pos >= overlap || buf == 0) {
                    outBuf1[i] = 0;
                    outPos1[i] = 0;
                    outXfade[i] = -1;
                } else {
                    outBuf1[i] = buf - 1;
                    outPos1[i] = bufspacing + pos;
                    outXfade[i] = 1 - (2 * pos / (overlap - 1));
                }
            } else {
                outBuf1[i] = buf;
                outPos1[i] = pos;
                if (pos >= overlap) {
                    outBuf0[i] = 0;
                    outPos0[i] = 0;
                    outXfade[i] = 1;
                } else {
                    outBuf0[i] = buf - 1;
                    outPos0[i] = bufspacing + pos;
                    outXfade[i] = (2 * pos / (overlap - 1)) - 1;
                }
            }
            outPlaying[i] = (playing == 0);

            if (playing == -1 && rate > 0) {
                playing = 0;
            }

            if (playing == 1 && rate < 0) {
                playing = 0;
            }

            if (playing == 0) {
                pos += rate;
                if (pos >= bufspacing) {
                    pos -= bufspacing;
                    buf++;
                }
                if (pos < 0) {
                    pos += bufspacing;
                    buf--;
                }
            }

            if ((buf == endbuf && pos >= endpos) || buf > endbuf) {
                if (loop) {
                    // TODO: this needs work
                    buf = beginbuf;
                    pos = pos - endpos + beginpos;
                } else {
                    buf = endbuf;
                    pos = endpos;
                    playing = 1;
                }
            }

            if ((buf == beginbuf && pos <= beginpos) || buf < beginbuf) {
                if (loop) {
                    // TODO: this needs work
                    buf = endbuf;
                    pos = pos - beginpos + endpos;
                } else {
                    buf = beginbuf;
                    pos = beginpos;
                    playing = -1;
                }
            }
        }

        // store back to the struct
        mPrevtrig = trig;
        mBuf = buf;
        mPos = pos;
        mPlaying = playing;
    }

    //////////////////////////////////////////////////////////////////

    // calculation function for audio rate rate
    void next_ka(int inNumSamples)
    {

    }

    //////////////////////////////////////////////////////////////////

    // calculation function for audio rate trig
    void next_ak(int inNumSamples)
    {

    }

    //////////////////////////////////////////////////////////////////

    // calculation function for audio rate trig and rate
    void next_aa(int inNumSamples)
    {

    }
};

// the entry point is called by the host when the plug-in is loaded
PluginLoad(MegaPhasorUGens)
{
    // InterfaceTable *inTable implicitly given as argument to the load function
    ft = inTable; // store pointer to InterfaceTable

    // registerUnit takes the place of the Define*Unit functions. It automatically checks for the presence of a
    // destructor function.
    // However, it does not seem to be possible to disable buffer aliasing with the C++ header.
    registerUnit<MegaPhasor>(ft, "MegaPhasor");
}
