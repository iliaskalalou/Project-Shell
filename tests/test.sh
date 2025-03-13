#!/bin/sh

executable="../src/42sh"
ref="bash"
nb_test=0
failed_test=0
passed_test=0
A=actual
E=expected

green_color="\033[32m"
red_color="\033[31m"
default_color="\033[00m"

print_ok()
{
	echo -e "[${green_color}OK${default_color}] $1"
}

print_fail()
{
	echo -e "[${red_color}FAIL${default_color}] $1:\n$2"
}

print_result()
{
	echo -e "${nb_test} Tests"
	echo -e "\t${green_color}${passed_test}${default_color} passed"
	echo -e "\t${red_color}${failed_test}${default_color} failed"
}

cleanup()
{
    rm -rf test_dir
	rm -f /tmp/{actual,exp}_{err,out}.txt
	rm -f /tmp/res_diff.txt
    rm -f /tmp/mem_err.txt
    rm -rf /tmp/$A
    rm -rf /tmp/$E
}

print_test()
{
	echo -e "\n------------------------------------"
	echo -e "\t$1"
	echo -e "------------------------------------\n"
}

compare_file()
{
	diff -u $2 $3 > /tmp/res_diff.txt 2>&1
	if [ $? -ne 0 ]; then
		print_fail "$1" "$(cat /tmp/res_diff.txt)"
		return 1
	fi
	return 0
}

compare_error()
{
    exp=$(cat $2)
    actual=$(cat $3)
    if [ ! -z "$exp" ]; then
        if [ -z "$actual" ]; then
            print_fail "$1" "Expected something on stderr"
            return 1
        fi
    else
        if [ ! -z "$actual" ]; then
            print_fail "$1" "Expected nothing on stderr"
            return 1
        fi
    fi
    return 0
}

compare_redir()
{
    for file in $2; do
        compare_file $1 $3/$file $4/$file
        if [ $? -ne 0 ]; then
            return 1
        fi
    done
    return 0
}

check_memory()
{
#    t=$(cat /tmp/mem_err.txt | grep -E "(LEAK)|(Invalid)")
    if [ ! -z "$t" ]; then
        print_fail "$1" "$(cat /tmp/mem_err.txt)"
         return 1
    fi
    return 0
}

test_case()
{
	nb_test=$((nb_test+1))
	# expected output
	#
    if [ $1 = "-p" ]; then
        $($2 | $ref) 1>/tmp/exp_out.txt 2>/tmp/exp_err.txt
	    exp_ret=$?

        $($2 | ./${executable}) 1>/tmp/actual_out.txt 2>/tmp/actual_err.txt
	    actual_ret=$?
    elif [ $1 = "-r" ]; then
        $($ref $1 "$2") 1>/tmp/exp_out.txt 2>/tmp/exp_err.txt
	    exp_ret=$?

        $(./${executable} $1 "$2") 1>/tmp/actual_out.txt 2>/tmp/actual_err.txt
#	    valgrind ./${executable} $1 "$2" >/tmp/tmp.txt 2>/tmp/mem_err.txt
	    actual_ret=$?
    else
	    ${ref} $1 "$2" 1>/tmp/exp_out.txt 2>/tmp/exp_err.txt
	    exp_ret=$?
	    # actual output

	    ./${executable} $1 "$2" 1>/tmp/actual_out.txt 2>/tmp/actual_err.txt
#	    valgrind ./${executable} $1 "$2" >/tmp/tmp.txt 2>/tmp/mem_err.txt
	    actual_ret=$?
    fi
	if [ -n "$(grep ERROR /tmp/actual_err.txt)" ]; then
		print_fail "$*"
		cat /tmp/actual_err.txt
		failed_test=$((failed_test + 1))
		return
	fi

	# compare stderr
	compare_error "$*" /tmp/exp_err.txt  /tmp/actual_err.txt
	if [ $? -eq 0 ]; then
		# compare stdout
		compare_file "$*" /tmp/actual_out.txt /tmp/exp_out.txt
		if [ $? -eq 0 ]; then
            # compare redir
            if [ $1 = "-r" ]; then
                compare_redir "$*" "0 1 2 3 4 5 6 7 8 9" $A $E
            fi
            if [ $? -eq 0 ]; then
               # check check_memory
                check_memory "$*"
                if [ $? -eq 0 ]; then
	    	    	# compare return value
		    	    if [ ${actual_ret} -eq ${exp_ret} ]; then
			    	    print_ok "$*"
    				    passed_test=$((passed_test+1))
     	    			return
	        		else
		        		print_fail "$*" "Expected return code ${exp_ret}, got ${actual_ret}"
			        fi
                fi
            fi
		fi
	fi
	failed_test=$((failed_test+1))
}

#print_test "Simple command"

test_case -c "true"
test_case -c "false"
test_case -c "echo test"
test_case -c "echo 'test'"
test_case -c "echo two; echo input"
test_case -c "echo 'two'; echo 'input'"
test_case -c "echo 'two' 'test' 'test2'"
test_case -c "echo "two" "test" "test2""
test_case -c "echo 'two test test2'"
test_case -c "iop"
test_case -c "echo test # ceci est un test"

#print_test "Simple if"

test_case -c "if true; then echo oui; else echo non; fi"
test_case -c "if false; then echo oui; else echo non; fi"
test_case -c "if true; then echo oui; else echo non; "

#print_test "Builtin"

test_case -c "echo -E '78\nhj'"
test_case -c "echo -Ee '\\\\'"
test_case -c "echo -e '456\nuoi'"
test_case -c "echo -n test"
test_case -c "echo   test	 test2      test3 	 test4 "
test_case -c "echo -e '78\top'"
test_case -c "if 'true' ; then echo yep, still works; fi"

#print_test "Exec"
test_case -c "seq 5"


#print_test "Tests des Commandes Conditionnelles"

test_case -c "if true; then echo oui; else echo non; fi"

test_case -c "if false; then echo oui; else echo non; fi"

test_case -c "if true; then echo oui; else echo non; "

test_case -c "if false; then echo non; elif true; then echo oui; else echo maybe; fi"

test_case -c "if false; then echo non; elif false; then echo peut-être; else echo oui; fi"

test_case -c "if true; then if false; then echo non; else echo oui; fi; else echo maybe; fi"

#print_test "Tests des Builtins"

test_case -c "echo test"

test_case -c "echo -E '78\nhj'"

test_case -c "echo -e '456\nuoi'"

test_case -c "echo -n test"

test_case -c "echo   test	 test2      test3 	 test4 "

test_case -c "echo -e '78\top'"

test_case -c "if 'true' ; then echo yep, still works; fi"

#print_test "Tests des Listes de Commandes"

test_case -c "echo one; echo two; echo three"

test_case -c "echo one; echo two; echo three;"

test_case -c "echo one
echo two
echo three"

test_case -c "echo one; echo two
echo three; echo four"

#print_test "Tests des Redirections"

test_case -c "cat test.sh"

#print_test "Tests des Citations Simples"

test_case -c "echo 'test'"

test_case -c "echo 'two' 'test' 'test2'"

test_case -c "echo 'two test test2'"

#print_test "Tests des Commentaires"

test_case -c "echo test # ceci est un test"

test_case -c "echo hello # comment || | & &&"

test_case -c "# echo should not run"

test_case -c "echo not#first #commented"

test_case -c "echo '#notacomment' #comment"

#print_test "Tests des Erreurs et des Statuts de Sortie"

test_case -c "foobar"

test_case -c "if true; then echo oui; fi else echo non;"

test_case -c "echo 'test'"

test_case -c "if true; then echo oui; else"

#print_test "Tests des Options de Commande"

test_case -c "echo Input as string"

test_case -c "echo one; echo two"

test_case -c "if true; then echo yes; else echo no; fi"

test_case -c "echo if then else"

test_case -c "if false ; then /bin/echo false ; fi"

test_case -c "if /bin/false ; then /bin/echo false ; else /bin/echo rr fi"

#print_test "Tests des Entrées via Fichier et Stdin"

test_case scripts/script1.sh

test_case scripts/script2.sh

test_case scripts/script3.sh

test_case scripts/script4.sh

test_case scripts/script5.sh

test_case scripts/script6.sh

test_case scripts/script7.sh

test_case scripts/script8.sh

test_case scripts/script9.sh

test_case scripts/script10.sh

test_case scripts/script11.sh

#print_test "Tests des pipelines"

test_case -c "true | false"

test_case -c "false | true"

test_case -c "true | false |true"

test_case -c "false | true | false"

test_case -c "true | false | false | true"

test_case -c "cat file.txt | wc -l"

test_case -c "if ! true; then echo yes; else echo no; fi"

test_case -c "while false; do echo yes; done"

test_case -c " until echo 2; do echo no; done"

test_case -c "while false; do echo yes; done; echo while false do echo nan done;"

test_case -c "echo true | echo false"

test_case -c "echo true || echo false"

test_case -c "echo true && echo false"

test_case -c "echo true \&  \| echo false"

test_case -c "echo `date`"

test_case -c "echo 'Lorem' 'ipsum'  'dolor' 'sit' 'amet'',' 'consectetur'"

test_case -c "echo 'Lorem' 'ipsum'''  ''dolor'' 'sit' 'amet'',' 'consectetur'"

test_case -c "echo 'Lorem' 'ipsum''  ''dolor'' 'sit' 'amet'',' 'consectetur'"

test_case -c "echo 'Lorem' \"ipsum\" \"dolor\" 'sit' \"amet\"\",\" 'consectetur'"

test_case -c "echo true \&  \| echo false"

test_case -c "echo -n please do not add a newline"

test_case -c "/bin/echo Hello ; if /bin/true; then /bin/echo inside if; fi; /bin/echo Good Bye!"

test_case -c "if /bin/echo cond then /bin/echo thenclause ; fi"

test_case -c "echo \"eeee\"'"

test_case -c "echo \"\\\"eeee\\\"'\""

test_case -c "echo \cont\
	ent"

test_case test_files/double1.sh

test_case test_files/double2.sh

test_case test_files/double3.sh

test_case test_files/double4.sh

test_case test_files/escape1.sh

test_case test_files/escape2.sh

test_case test_files/escape3.sh

test_case test_files/escape4.sh

test_case test_files/escape5.sh

test_case test_files/escape6.sh

test_case test_files/escape7.sh

test_case test_files/escape8.sh

test_case test_files/escape9.sh

test_case test_files/escape10.sh

test_case test_files/escape11.sh

test_case test_files/escape12.sh

test_case test_files/escape13.sh

test_case -c "while false; do echo yes done"

test_case -c " until echo 2; do echo no done"

test_case -c "while false; do echo yes; "

test_case -c " until echo 2 do echo no; done"


test_case -c "while false; do ; done"

test_case -c " until ; do echo no; done"

test_case -c " until"

test_case -c " while false; do  done"

test_case -c " until ; do ;done"

test_case -c " until ; done"

test_case -c "a=10"

test_case -c "a= 10"

test_case -c "a="

test_case -c "a=10; echo \$a"

test_case -c "var=10; var=20; echo \${var}"

test_case -c "var=10 echo \${var}"

test_case -p "cat test_files/escape1.sh"

test_case -c "var='Kaealag'; echo \$var"

test_case test_files/variables1.sh

test_case test_files/variables2.sh

test_case -c "var=sdk?; echo \$var"

test_case -c "var1=8; var2=\$var; echo \$var2"

test_case -c "if true; then var1=0; echo \$var1; var2=1; echo \$var2; else echo faux; fi"

test_case -c "var=\"Doubles quotes\"; echo \$var"

test_case -c "echo $RANDOM"

test_case -c "echo \$vide"

test_case -c "echo rrrr'trtrt'r"

test_case -c "var=10 ; echo ttt$var'yyy'ttt"

test_case -c "for i; do echo test; done"

test_case -c "for i in 1; do echo test; done"

test_case -c "for i in 1; do echo \$i; done"

test_case -c "for i in 1 2 3; do echo \$i; done"

test_case -c "''"

test_case -c "echo \\ ''"

test_case test_files/for1.sh

test_case test_files/for2.sh

test_case test_files/for3.sh


test_case test_files/for5.sh

test_case test_files/for6.sh

test_case -c "if false; then echo BAD; elif true; then echo 'Can finally respect 80 cols...'; else echo BAD; fi"

test_case -c "echo 'Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Egestas purus viverra accumsan in nisl nisi. At quis risus sed vulputate. Neque laoreet suspendisse interdum consectetur libero. Et molestie ac feugiat sed lectus vestibulum mattis. Tristique nulla aliquet enim tortor at auctor. Aliquet porttitor lacus luctus accumsan tortor. Tellus cras adipiscing enim eu turpis. Mattis ullamcorper velit sed ullamcorper morbi tincidunt ornare. Nec sagittis aliquam malesuada bibendum arcu vitae elementum. Consequat id porta nibh venenatis cras. Dolor magna eget est lorem ipsum. Vivamus at augue eget arcu dictum varius duis. Aliquam eleifend mi in nulla. Cursus risus at ultrices mi tempus imperdiet nulla malesuada. Adipiscing elit ut aliquam purus sit amet luctus venenatis. Risus at ultrices mi tempus imperdiet nulla malesuada pellentesque elit. Mauris cursus mattis molestie a iaculis at erat pellentesque adipiscing.'"

test_case -c "echo -e -e '=> \\\\'"

test_case -c "echo -E -e -E '\\\\'"

test_case -c "echo -e -e '=> \\\\'"

test_case -c "echo -e \\\\"

test_case -c "echo Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Egestas purus viverra accumsan in nisl nisi. At quis risus sed vulputate. Neque laoreet suspendisse interdum consectetur libero. Et molestie ac feugiat sed lectus vestibulum mattis. Tristique nulla aliquet enim tortor at auctor. Aliquet porttitor lacus luctus accumsan tortor. Tellus cras adipiscing enim eu turpis. Mattis ullamcorper velit sed ullamcorper morbi tincidunt ornare. Nec sagittis aliquam malesuada bibendum arcu vitae elementum. Consequat id porta nibh venenatis cras. Dolor magna eget est lorem ipsum. Vivamus at augue eget arcu dictum varius duis. Aliquam eleifend mi in nulla. Cursus risus at ultrices mi tempus imperdiet nulla malesuada. Adipiscing elit ut aliquam purus sit amet luctus venenatis. Risus at ultrices mi tempus imperdiet nulla malesuada pellentesque elit. Mauris cursus mattis molestie a iaculis at erat pellentesque adipiscing."

test_case test_files/step2_1.sh

test_case test_files/step2_2.sh

test_case test_files/escape14.sh

test_case test_files/escape15.sh

test_case test_files/escape16.sh

test_case test_files/escape17.sh

test_case test_files/escape18.sh

test_case test_files/escape19.sh

test_case -c "echo 'g gerhg erhterh ''"

test_case -c "echo evg; exit 2; echo gwerg"

test_case -c "echo gerhe ;exit "

test_case -c "echo \$PWD; cd src; echo \$PWD"

test_case -c "echo \$OLDPWD; cd src; echo \$OLDPWD"

test_case -c "var=
echo \$var po"

test_case -c ". ./tests/scripts/script1.sh"

test_case -c "echo '#quoted'"

test_case -c "/bin/echo '#'quoted"

test_case -c "echo '#'"

test_case -c "/bin/echo #quoted'gegergr'"

test_case -c "/bin/echo '#quoted' 'gegergr'"

test_case -c "/bin/echo #quoted\"gegergr\""

test_case -c "/bin/echo 'unclosed single quote"

test_case -c "echo 'ls'test'fergre'"

test_case -c "echo '`date`'"

test_case -c "var=
echo \$var test"

test_case -c "''"

test_case -c "'"

test_case -c "'''"

test_case -c "/bin/echo \"bad line"

test_case -c "/bin/echo 'bad' ';' 'line''bad' ';' 'line''bad' ';' 'line' 'bad' ';' 'line' 'bad' ';' 'line' 'bad' ';' 'line' 'bad' ';' 'line' "

test_case -c "/bin/echo ''then' 'echo' 'cond'';' 'echo' 'nah''; fi'"

test_case -c "if true; then /bin/echo 'cond'; /bin/echo 'nah'; fi"

test_case -c "if echo 'cond'; then 'nah';"

test_case -c "if echo 'cond'; then 'nah'; else echo feur ;fi"

test_case -c "echo 3 2 && echo 5 3 || echo tygrg"

test_case -c "\${seb}\${tu} est beau"

test_case -c "$"

test_case -c "b='le 42'
takebon \${b} sh"

test_case -c "echo rferg || "

test_case -c "echo freg && || echo grer e"

test_case -c "echo rr; echo fefe; echo gerger && echo grg; echo ferge"

test_case -c "echo rr; echo fefe || echo gerger && echo grg && echo ferge"

test_case -c "/bin/echo \"cou\\\"\"\"cou\\\"c est moi\""

test_case -c "echo \\\$te*st"

test_case -c "echo \$te*st"

test_case -c "var*=1 "

test_case -c "echo \% \# \^ \\ \: \;"

test_case -c "for test
do
	echo done
done"

test_case -c "if !! echo true; then echo dalse; fi "

test_case -c "! echo true "

test_case -c "! echo true !"

test_case -c "echo \"roger,  bois ton ricard !\"
echo \"dis donc roger tes bourre ou quoi\""


test_case -c "if  ; then ;"

test_case -c "if  echo oui; then ;"

#test_case -c "while true ; do echo no; done"

test_case -c ""
test_case -c "
"

test_case -c "if"

test_case -c "ls ''../src"

test_case -c "echo \`date\`"

test_case -c "echo yes > feur"

test_case -c "echo yes 1> feur"

test_case -c "echo yes || echo feur"

test_case -c "echo yes || echo \"feur\""

test_case -c "echo \$PWD; cd /; cd -; echo \$PWD"

test_case -c "echo \$PWD; cd /; cd ../; cd - ; echo \$PWD"

test_case -c "echo \PWD; cd; echo \$PWD"

test_case -c "echo yes > feur"

test_case -c "echo yes >> feur"

test_case -c "echo yes >| feur"

test_case -c "echo yes <> feur"

test_case -c "echo yes 1> feur"

test_case -c "echo yes; egr"

test_case -c " ;   "

test_case -c " 



"

test_case -c " echo before disaster ...;;
after disaster ..."

test_case -c " echo ttt |"



print_result









