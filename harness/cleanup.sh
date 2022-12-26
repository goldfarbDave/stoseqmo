# So first we grepped all of the header lines with
# grep -n "shakespeare" incrementalshakespeare_other.csv | cut -d':' -f1
# Then did a bit of python to get the size of each chunk
# for a, b in zip(li, li[1:]):
#     print(b-a)
# Then did a chunk invocation (Note the `tail' is needed to remove the header schema)
# tail -n +2  incrementalshakespeare_other.csv | split --lines=5736239 - outchunk
# Zip it with pigz
# tar cvf - tmpoutchunks/ | pigz > outchunks.tar.gz
# scp it locally, unzip
