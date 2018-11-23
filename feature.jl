import PyCall

cv2 = PyCall.pyimport("cv2") 


img_path = raw"C:\zsz\ML\code\DL\img_tools\test\img\2\2.jpg"
img = cv2[:imread](img_path)
println(size(img)) 

sift = cv2[:xfeatures2d.SIFT_create]()
println(sift)



