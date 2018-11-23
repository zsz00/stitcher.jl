# test images
import Images, ImageFeatures, FileIO, ImageView, Dates
# using CoordinateTransformations, StaticArrays, ImageTransformations, LinearAlgebra;

time1 = Dates.now()
img1 = FileIO.load("C:/Users/m1/test/src/imgs/4.jpg")  
# [win10,小米笔记本, img:1280*720] no ImageMagick used 7s, 2018.11.23
# img1 = Images.ImageMeta(img1, time="aaa")
# println(img1["time"])
# print("used: ", convert(Float64, Dates.now()-time1))
print("used: ", (Dates.now()-time1).value/1000, " s")

