# tu yang camera test

mutable struct TY_VERSION_INFO{T1}
    major::T1
    minor::T1
    patch::T1
    reserved::T1
end

a = TY_VERSION_INFO(0,0,1,0)
println(a.minor)

t = ccall((:TYLibVersion, "libtycam.so"), Int32, (a,))
println(t)
println(a)
   
