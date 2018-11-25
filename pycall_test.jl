# test PyCall
using PyCall, FileIO, ImageView
# using Conda
# Conda.add("matplotlib")
@pyimport numpy as np
# @pyimport matplotlib.pyplot as plt
# @pyimport cv2 as cv2
cv2 = pyimport("cv2") 

img_path = raw"C:\zsz\ML\code\DL\img_tools\test\img\2\2.jpg"
# img = cv2[:imread](img_path)
img = load(img_path)
# img = np.ndarray(img)  # Array{PyObject,2}   Array{ColorTypes.RGB{FixedPointNumbers.Normed{UInt8,8}},2}
println(typeof(img))
# cv2[:imshow]("raw", img); cv2[:waitKey]()

sift = cv2[:xfeatures2d][:SIFT_create]()


function get_features(img)
    img = PyArray(img)
    gray = cv2[:cvtColor](img, cv2[:COLOR_BGR2GRAY])
    # keypoints, descriptors = surf.detectAndCompute(gray, None)
    keypoints, descriptors = sift[:detectAndCompute](gray, nothing)
    println(size(descriptors))

end

# get_features(img)


# @pyimport numpy as np
# @pyimport matplotlib.pyplot as plt
# # @pyimport 'cv2.cv2' as cv2
# cv2 = PyCall.pyimport("cv2")

# img_path = "test/16.jpg"
# img = cv2[:imread](img_path)
# plt.imshow(img); plt.show()   # WIN10 上显示有问题, 弹Qt的报错.

# PyCall.PyArray 将Numpy的多维数组(ndarray)转换为Julia的Array类型