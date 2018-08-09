// without mul and add.
MegaPhasor : MultiOutUGen {
    *ar { arg numbufs, overlap=5, trig=0, rate=1, startbuf=0, startpos=0, beginbuf=0, beginpos=0, endbuf=0, endpos=0, loop=0;
        ^this.multiNew('audio', numbufs, overlap, trig, rate, startbuf, startpos, beginbuf, beginpos, endbuf, endpos, loop)
    }
    init { arg ... theInputs;
        inputs = theInputs;
        ^this.initOutputs(6, rate);
    }
}
