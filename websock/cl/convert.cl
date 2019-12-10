float clip(float v)
{
    if(v > 255)
        return 255;
    if(v < 0)
        return 0;
    return v;
}

float3 getRgb(float3 yuv)
{
    float3 vec;

    vec.x = yuv.x + 1.402 * (yuv.z - 128);
    vec.y = yuv.x - 0.344 * (yuv.y - 128) - 0.714 * (yuv.z - 128);
    vec.z = yuv.x + 1.772 * (yuv.y - 128);

    vec.x = clip(vec.x);
    vec.y = clip(vec.y);
    vec.z = clip(vec.z);
    return vec;
}

__kernel void convert(__global uchar* Y, __global uchar* U, __global uchar *V,
                    __global uchar* Rgb, int lsY, int lsU, int lsV, int lsRgb,
                     int width, int height)
{
    int gid = get_global_id(0);
    int x = gid % width;
    int y = gid / width;

    int y2 = (int)(y/2.);
    int x2 = (int)(x/2.);

    int _y = Y[lsY * y + x];
    int _u = U[lsU * y2 + x2];
    int _v = V[lsV * y2 + x2];

    float3 _rgb;

    _rgb = getRgb((float3)(_y, _u, _v));

    Rgb[lsRgb * y + x * 4 + 2] = _rgb.x;
    Rgb[lsRgb * y + x * 4 + 1] = _rgb.y;
    Rgb[lsRgb * y + x * 4 + 0] = _rgb.z;
}
