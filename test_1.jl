import Dates

# println(Dates.now())
# println(Dates.format(Dates.now(), "yyyy-mm-dd HH:MM:SS.s"))

ss = 1
for i in 1:10
    global ss += 1
end
println(ss)
