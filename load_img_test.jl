# test image read ,  ssim
using Images  #, ImageView

function ssim(img1, img2)
    im1 = load(img1);
    im1 = Gray.(im1);
    im1 = imresize(im1, (300, 300));
    im2 = load(img2);
    im2 = Gray.(im2);
    im2 = imresize(im2, (300, 300));
    return 0
#     imshow(im1)
end


img1 = raw"C:\zsz\ML\code\DL\img_tools\test\img\2\2.jpg"
img2 = raw"C:\zsz\ML\code\DL\img_tools\test\img\2\00025.jpg"
# img1 = "/root/julia/imgs/00000000210000000_02868.jpg"  # stata-1.png
# img2 = "/root/julia/imgs/00000000210000000_02928.jpg"

@time for i in 1:10
    ssim(img1, img2)
end


#=
python3 : time: 0.19165117740631105 s

@time ssim(img1, img2)
  0.978499 seconds (551 allocations: 143.073 MiB, 11.40% gc time)
绝大部分是 load的时间.
读图片 julia Images 比 python opencv 慢5倍.  test on ubuntu16.04 阿里云

time: 0.03723652362823486 s
time: 0.06383512020111085 s

0.6096748

=#

