package com.kalyzee.rctgstplayer.utils;

public class RCTGstAudioLevel {
    private double rms;
    private double peak;
    private double decay;

    public RCTGstAudioLevel(double rms, double peak, double decay) {
        this.rms = rms;
        this.peak = peak;
        this.decay = decay;
    }

    public double getRms() {
        return this.rms;
    }

    public double getPeak() {
        return this.peak;
    }

    public double getDecay() {
        return this.decay;
    }

    @Override
    public String toString() {
        return "RCTGstAudioLevel - RMS : " + this.getRms() + " - Peak : " + this.getPeak() + " - Decay : " + this.getDecay();
    }
}
