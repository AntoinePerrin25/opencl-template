kernel void my_sqrt(__global float *in, __global float *out) {
    int id = get_global_id(0);
    out[id] = sqrt(in[id]);
}