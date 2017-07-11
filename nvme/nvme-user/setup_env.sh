VER1=`uname -r | sed 's/-generic//g'`
echo ${VER1}
#VER2=`echo ${VER1}`
export KERNEL_VER="${VER1}"
echo ${KERNEL_VER}
