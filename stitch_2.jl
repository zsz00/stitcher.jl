# stitcher 
using Images, ImageFeatures, FileIO, ImageView, ImageDraw, Colors

img1 = load("test/imgs/1.jpg")
img2 = load("test/imgs/2.jpg")
# imshow(img1)

function get_descriptors(img::AbstractArray)
    imgp = parent(img)
    brisk_params = ImageFeatures.BRISK()
    features = ImageFeatures.Features(Keypoints(imcorner(imgp, method=harris)))  # harris  orb sift  hog  这些都没有
    desc, ret_features = ImageFeatures.create_descriptor(Gray.(imgp), features, brisk_params)
end


function match_points(img1::AbstractArray, img2::AbstractArray, threshold::Float64=0.1)
    img1p = parent(img1)
    img2p = parent(img2)
    desc_1, ret_features_1 = get_descriptors(img1p)
    desc_2, ret_features_2 = get_descriptors(img2p)
    matches = ImageFeatures.match_keypoints(
        Keypoints(ret_features_1), Keypoints(ret_features_2), desc_1, desc_2, threshold)
end


# this function takes the two images and concatenates them horizontally.
# to horizontally concatenate, both images need to be made the same vertical size
function pad_display(img1, img2)
    img1h = length(axes(img1, 1))
    img2h = length(axes(img2, 1))
    mx = max(img1h, img2h);
    grid = hcat(vcat(img1, zeros(RGB{Float64}, max(0, mx - img1h), length(axes(img1, 2)))),
        vcat(img2, zeros(RGB{Float64}, max(0, mx - img2h), length(axes(img2, 2)))))
end


function draw_matches(img1, img2, matches)
    # instead of having grid = [img1 img2], we'll use the new pad_display() function
    grid = pad_display(parent(img1), parent(img2));
    offset = CartesianIndex(0, size(img1, 2));
    println(offset, length(axes(img1, 2)))
    for m in matches
        ImageDraw.draw!(grid, LineSegment(m[1], m[2] + offset))   # , color=colorant"red"
    end
    grid
end


using CoordinateTransformations, StaticArrays, ImageTransformations, LinearAlgebra;

struct Homography{T} <: AbstractAffineMap
    m::SMatrix{3, 3, T, 9}
end


function compute_homography(matches::Array{Keypoints})
    # eigenvector of A^T A with the smallest eigenvalue construct A matrix
    A = zeros(2 * length(matches), 9)
    for (index, match) in enumerate(matches)
        match1, match2 = match
        base_index_x = index * 2 - 1
        base_index_y = 1:3
        A[base_index_x, base_index_y] = float([match1.I...; 1;])
        A[base_index_x + 1, 4:6] = A[base_index_x, base_index_y]
        A[base_index_x, 7:9] = -1.0 * A[base_index_x, base_index_y] * match2.I[1]
        A[base_index_x + 1, 7:9] = -1.0 * A[base_index_x, base_index_y] * match2.I[2]
    end
  
    # find the smallest eigenvector, normalize, and reshape
    U, S, Vt = LinearAlgebra.svd(A)
    
    # normalize the homography at the end, since we know the (3, 3) entry should be 1.
    H = Homography{Float64}(reshape(Vt[:, end] ./ Vt[end][end], (3, 3))')  #  
end


matches = match_points(img1, img2, 0.1)   # OK,  匹配的不好
grid = draw_matches(img1, img2, matches)

H_computed_rot = compute_homography(matches)  # get homography matrix

img1_warp = AffineMap(img1, H_computed_rot.m);

img1_warp = ImageTransformations.warp(img1rot, H_computed_rot.m)  # ???

