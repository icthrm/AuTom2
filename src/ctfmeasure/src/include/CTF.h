class CTF
{
public:
    CTF();
    CTF(int n_now);
    ~CTF();

public:
    void setAllCTFPara(float defocus1_now,float defocus2_now,float astig_now,float phase_shift_now,float w_now);
    void setAllCTFEnvelopePara(float D_now,float alpha_now,float tilt_x_now,float tilt_y_now,float bfactor_now);
    void setAllImagePara(float pix_now,float volt_now,float Cs_now);
    void setN(int n_now);

    float getDefocus1();
    float getDefocus2();
    float getAstigmatism();
    float getPhaseShift();
    float getW();
    float getW_phase();
    float getPixelSize();
    float getVoltage();
    float getCs();
    float getLambda();
    int getN();

    float computeCTF2D(float x,float y,int Nx,int Ny,bool phaseflip,bool flip_contrast,float z_offset);
    float computeCTFEnvelope2D(float x,float y,int Nx,int Ny,float z_offset);

private:
    float defocus1,defocus2;    // in m
    float astig;    // in radius
    float phase_shift;  // in radius
    float w_cos,w_sin;
    float w_phase;

    float pix;  // in m
    float volt; // in V
    float Cs;   // in m
    float lambda;   // in m

    // parameters for envelope
    float D;    // defocus spread, in m
    float alpha;    // beam angle, in radius
    float tilt_x,tilt_y;    // beam tilt, in radius
    float bfactor;  // B-factor, in m^2

    int n;

};
