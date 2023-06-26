#!/bin/bash
export LANG=C
export LC_CTYPE=C
export LC_ALL=C

cd $(dirname "$0")

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
mv fn_log.h ./dist/include/fn-log/fn_log.h
cp README.md ./dist/include/fn-log/README.md 
cp LICENSE ./dist/include/fn-log/LICENSE 



last_sha1=`git rev-parse HEAD`
last_date=`git show --pretty=format:"%ci" | head -1`
last_diff=`git log -1 --stat `

last_dist_sha1=`git log -1 --stat ./src |grep -E "commit ([0-9a-f]*)" |grep -E -o "[0-9a-f]{10,}"`
last_dist_date=`git show $last_dist_sha1 --pretty=format:"%ci" | head -1`
last_dist_diff=`git log -1 --stat ./src`

echo ""
echo "[master last commit]:"
echo $last_sha1
echo $last_date
echo ""
echo "[master last diff]:"
echo $last_diff


echo ""
echo "[./src last commit]:"
echo $last_dist_sha1
echo $last_dist_date
echo ""
echo "[./src last diff]:"
echo $last_dist_diff

echo ""
echo "[write versions]"
echo "version:" > ./dist/include/fn-log/VERSION
echo "last_sha1(./src)=$last_dist_sha1" >> ./dist/include/fn-log/VERSION 
echo "last_date(./src)=$last_dist_date" >> ./dist/include/fn-log/VERSION 
echo "" >> ./dist/include/fn-log/VERSION 
echo "git log -1 --stat ./src:" >> ./dist/include/fn-log/VERSION 
echo $last_dist_diff >> ./dist/include/fn-log/VERSION
cat ./dist/include/fn-log/VERSION

echo ""
echo "[write done]"




