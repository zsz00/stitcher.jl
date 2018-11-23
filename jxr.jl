using FileIO


function aa_1(jxr)
    for i in range(3)
        if (jxr_val[i] <= 3000)
            col_val[i] = jxr_val[i] * 8e-8
        else
            col_val[i] = 0.00024 * 1.0002 ** (jxr_val[i] - 3000)


    col_val[0] *= 0.8
    col_val[1] *= 1.0
    col_val[2] *= 1.6


f1 = load("stitcher/test/j0_0.jxr")
# aa_1(f1)

