#genera file posizioni casuale
# $RANDOM is an internal Bash function (not a constant) that returns a pseudorandom integer in the range 0 - 32767. $RANDOM should not be used to generate an encryption key.

SIZE=5+1 #dimensione matrice + 1

number=0   #initialize

number=$RANDOM
let "number %= $SIZE"  # Scales $number down within $RANGE.

echo "Random number  $number"
echo
