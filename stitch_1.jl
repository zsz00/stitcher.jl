# stitcher: 使用外参 进行拼接, 图片间没有重合区域, 不用特征匹配求H.
using Images, ImageFeatures, FileIO, ImageView
using CoordinateTransformations, StaticArrays, ImageTransformations, LinearAlgebra;


img1 = load("test/all_3/1.jpg")
img2 = load("test/all_3/2.jpg")


rot_1 = [0.999788   0.003629  -0.020254; 
         -0.003697  0.999988  -0.003359;  
         0.020242   0.003433  0.999789 ]
tran_1 = [23.244938;  -0.077094;  0.316536]
# Rodrigues转换, 向量和矩阵,把3*3的矩阵转为3*1的向量. 对应到 python的 cv2.Rodrigues(rvecs, None)  
rvecs = RodriguesVec(rot_1)  # 0.00339625, -0.0202495, -0.00366327        

# RotMatrix(-pi*0.01)
rotate = RotMatrix{2}(rotate)  

H = CoordinateTransformations.LinearMap(rotate)
# H = LinearMap(RotMatrix(-pi*0.01))
img1 = ImageTransformations.warp(img1, H)   # error

# img1 = img1[400:end, 1:2592]
# img2 = img2[1:end-300, :]

# img = vcat(img2, img1)  # 拼接


