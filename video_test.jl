# test video
using VideoIO, ImageView
# VideoIO 安装失败, 放弃 2018.10.5
# VideoIO.viewcam()  # open the camera

f = VideoIO.opencamera(0)

# One can seek to an arbitrary position in the video
seek(f,2.5)  ## The second parameter is the time in seconds and must be Float64
img = read(f)
canvas, _ = ImageView.view(img)

while !eof(f)
    read!(f, img)
    ImageView.imshow(canvas, img)
    #sleep(1/30)
end

