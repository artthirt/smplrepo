float clip(float v)
{
    if(v > 255)
        return 255;
    if(v < 0)
        return 0;
    return v;
}

void getRgb(float Y, float U, float V, float &R, float &G, float &B)
{
    R = Y + 1.13983 * V;
    G = Y - 0.39465 * U - 0.58060 * V;
    B = Y + 2.03211 * U;

    R = clip(R); 
    G = clip(G); 
    B = clip(B); 
}

__kernel void convert(__global uchar8* Y, __global uchar8* U, __global uchar8 *V,
                    __global uchar8* Rgb, int lsY, int lsU, int lsV, int lsRgb,
                     int width, int height)
{
    int gid = get_global_id(0);
    int x = gid % width;
    int y = gid / width;

    float y = Y[lsY * y + x];
    float u = U[lsU * y + x/2];
    float v = V[lsV * y + x/2];

    float r, g, b;

    getRgb(y, u, v, r, g, b);

    uchar8 *rgb = &Rgb[lsRgb * y + x * 4];
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}