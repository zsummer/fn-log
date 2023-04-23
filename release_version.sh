#!/bin/bash
export LANG=C
export LC_CTYPE=C
export LC_ALL=C

echo "#ifdef __GNUG__" > fn_log.h
echo "#pragma GCC push_options" >> fn_log.h
echo "#pragma GCC optimize (\"O2\")" >> fn_log.h
echo "#endif" >> fn_log.h
cat src/include/fn_file.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
cat src/include/fn_data.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
cat src/include/fn_parse.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
cat src/include/fn_fmt.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
cat src/include/fn_load.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
cat src/include/fn_out_file_device.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
cat src/include/fn_out_udp_device.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
cat src/include/fn_out_screen_device.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
cat src/include/fn_out_virtual_device.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
cat src/include/fn_channel.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
cat src/include/fn_core.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
cat src/include/fn_stream.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
cat src/include/fn_macro.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
cat src/include/fn_log.h  |sed `echo -e 's/\xEF\xBB\xBF//'` >> fn_log.h
echo "#ifdef __GNUG__" >> fn_log.h
echo "#pragma GCC pop_options" >> fn_log.h
echo "#endif" >> fn_log.h
cat fn_log.h | sed '/#include.*fn_/d' > fn_log.h.bak

mv fn_log.h.bak  fn_log.h
mv fn_log.h ./dist/fn_log.h
cp README.md ./dist/README.md 
cp LICENSE ./dist/LICENSE 

version=`date +"release version date:%Y-%m-%d %H:%M:%S"`
echo $version > ./dist/VERSION 
echo "" >> ./dist/VERSION 
echo "git log:" >> ./dist/VERSION 
git log -1 --stat >> ./dist/VERSION 