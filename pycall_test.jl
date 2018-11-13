# test PyCall
using PyCall
# using Conda
# Conda.add("matplotlib")
@pyimport numpy as np
@pyimport matplotlib.pyplot as plt
# @pyimport cv2 as cv2
cv2 = pyimport("cv2") 

img_path = raw"C:\zsz\ML\code\DL\img_tools\test\img\2\2.jpg"
img = cv2[:imread](img_path)
plt.imshow(img); plt.show()   # WIN10 上显示有问题, 弹Qt的报错.


# @pyimport numpy as np
# @pyimport matplotlib.pyplot as plt
# # @pyimport 'cv2.cv2' as cv2
# cv2 = PyCall.pyimport("cv2")

# img_path = "test/16.jpg"
# img = cv2[:imread](img_path)
# # plt.imshow(img); plt.show()

# cv2[:imshow]("raw", img); cv2[:waitKey]()

# plt[:imshow](img); plt[:show]()

