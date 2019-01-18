#!/bin/bash

if [ $# -ne 4 ]
then
  echo "Must give 4 arguments"
  exit
fi

if [ ! -d $1 ]
then
  echo "Directory: " $1 " does not exist"
  exit
fi

if [ ! -f $2 ]
then
  echo "File: " $2 " does not exist"
  exit
fi

if ! [[ "$3" =~ ^[1-9][0-9]*+$ ]]
then
  echo "Argument 3: " $3 " is not a valid number"
  exit
fi

if ! [[ "$4" =~ ^[1-9][0-9]*+$ ]]
then
  echo "Argument 3: " $4 " is not a valid number"
  exit
fi

lines=$(wc -l $2 | cut -f 1 -d " ")
if [ $lines -lt 10000 ]
then
  echo "File has less than 10000 lines,exactly: " $lines
  exit
fi

if [ "$(ls -A $1)" ]
then
  echo "# Warning: directory is full,purging ..."
  rm -rf ./root_dir/*
fi

array=()  #all sites

cd root_dir #access root_dir
for i in `seq 0 $(($3-1))`  #dhmiourgia twn katalogwn
do
  dir_name="site_"$i
  mkdir $dir_name
  cd $dir_name  #in site
  for j in `seq 1 $4`
  do
    x=$(((RANDOM % (32767+1-1000))+1000))  #4-5 digits
    page_name="page_"$x".html"
    while [ -f $page_name ] #an tuxei idios arithmos
    do
      x=$(((RANDOM % (32767+1-1000))+1000)) #rand%(max-min)+min->min-(max-1)
      page_name="page_"$x".html"
    done
    path="root_dir/"$dir_name"/"$page_name
    array+=($path)
    touch $page_name
  done
  cd ../  #out of site
done

f=$(($4/2 +1))
q=$(($3/2 +1))
group_amount=$((f+q))

incoming_links=()
for i in `seq 0 $(($3-1))`
do
  echo "# Creating web site" $i "..."
  dir="site_"$i
  cd $dir
  z=($(ls)) #array for page names
  all_external=()
  for j in "${array[@]}"
  do
    if [[ $j != *"$dir"* ]] #all_external exei ola ta pages ektos apo to site pou eimaste
    then
      all_external+=($j)
    fi
  done
  all_internal=()
  for j in "${z[@]}"  #all internal links
  do
    all_internal+=($j)
  done
  for j in "${z[@]}"
  do
    all_external=($(shuf -e ${all_external[@]}))  #all internal mixed
    all_internal=($(shuf -e ${all_internal[@]}))  #all internal mixed
    external=() #array with final externals
    for el in `seq 0 $(($q-1))`
    do
      str=${all_external[$el]}  #antikastastash tou root_dir me ..
      no_root=${str:8}
      external+=(".."$no_root)
    done
    internal=()
    for el in `seq 0 $f`  #elegxontai ta prwta f+1 pages,kratountai ta f egkura apo auta
    do                   #akuro einai auto pou exei to onoma tou page pou briskomaste
      if [ "${all_internal[$el]}" != "$j" ] && [ "${#internal[@]}" -lt "$f" ]
      then
        internal+=(${all_internal[$el]})
      fi
    done
    final_f_q=()  #all final external and internal links
    for el in "${external[@]}"
    do
      final_f_q+=($el)
    done
    for el in "${internal[@]}"
    do
      final_f_q+=($el)
    done
    final_f_q=($(shuf -e ${final_f_q[@]}))  #anakatema
    k=$(((RANDOM % ($lines-2000-2))+2)) #apo 2 mexri kai $lines-2000-1
    m=$(((RANDOM % (2000-1001))+1001))
    end=$((k+m))  #teleutai grammh toy plaisiou
    lines_group=$((m/(f+q)))  #plhthos link
    echo "#   Creating page root_dir/"$dir"/"$j with $m "lines starting at line" $k "..."
    echo "<!DOCTYPE html>
<html>
  <body>" >> $j
    next_line=k
    for l in `seq 1 $group_amount`  #gia kathe group grammwn
    do
      the_line=$((next_line+b))
      the_line_end=$((next_line+$lines_group))
      sed -n "${the_line},${the_line_end}p" ../../$2 >> $j
      if [[ ${final_f_q[(($l-1))]} = *"site"* ]]  #an einai external ginetai antikatastash tou .. me root_dir
      then
        aft=${final_f_q[(($l-1))]}
        str_path=${aft:2}
        path="root_dir"$str_path
      else  #an den einai external link ginetai prosthkh sthn arxh tou root_dir/sitei
        path="root_dir/"$dir"/"${final_f_q[(($l-1))]}
      fi
      incoming_links+=($path) #pinakas me ola ta pages pou exoun incoming_link
      echo "#     Adding link to" $path
      echo -e "<a href=\""${final_f_q[(($l-1))]}"\">$path</a>" >> $j
      next_line=$((next_line+lines_group))
    done
    first_l=$((next_line+1))
    sed -n "${first_l},${end}p" ../../$2 >> $j
  echo "  </body>
</html>" >> $j
  done
  cd ../
done

cd ../ #back to directory

touch tmp_file.txt
echo "" > tmp_file.txt  #delete content of previous file
for i in "${array[@]}"
do
  echo $i >> tmp_file.txt
done
array=($(sort -V tmp_file.txt | uniq ))
echo "" > tmp_file.txt
for i in "${incoming_links[@]}"
do
  echo $i >> tmp_file.txt
done
incoming_links=($(sort -V tmp_file.txt | uniq ))
rm -f tmp_file.txt

diff=$(diff <(printf "%s\n" "${array[@]}") <(printf "%s\n" "${incoming_links[@]}"))

if [[ -z "$diff" ]]
then
    echo "# All pages have at least one incoming link"
else
    echo "# NOT All pages have at least one incoming link"
fi
echo "# Done."
