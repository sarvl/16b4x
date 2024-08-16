" Vim syntax file
" Language:		sasm
" Maintainer:		sarvel
" Contributors:		sarvel
" Last Change:		2024 Aug 16

" quit when a syntax file was already loaded
if exists("b:current_syntax")
  finish
endif

let s:cpo_save = &cpo
set cpo&vim


syn match error "."

syn match separator ",\|\s"
syn match operator "+\|-\||" contained containedin=expression
syn match operator "/\|%\|&" contained containedin=expression
syn match operator "?\|!\|=" contained containedin=expression
syn match operator "#\|<\|>" contained containedin=expression
syn match operator ","       contained containedin=expression
syn match operator "\$\|\*\|\^\|\~\|\." contained containedin=expression

"labels
syn match label "[a-zA-Z][a-zA-Z0-9_-]*\(\.[a-zA-Z][a-zA-Z0-9_-]*\)\?" containedin=expression
syn match label "\.[a-zA-Z][a-zA-Z0-9_-]*" containedin=expression
syn match label "\.<\|\.>"
syn match labeldef "\(\.\)\?[a-zA-Z][a-zA-Z0-9_-]*:\|\.\.\|\.,"

syn match arglist "(\|)" containedin=expression
syn region expression start="{" end="}" containedin=expression contains=expression contains=operator

syn match error "%[^ \t})]\+" containedin=expression
"each directive also does simple error checking to see whether next argument is something that can be correct
"whether that argument is correct is checked by different part of syntax highlight
"grouped by argument types
"sorted alphabetically
"	string/'%tostring'
syn match directive "^\s*%\(error\|inc\|info\|warn\|wes\|wns\|ws\)\ze\s\+\(\"\|%tostring\)"
"	number/variable/define/expression
syn match directive "^\s*%\(adr\|align\|elsif\|if\|rps\|\)\ze\s\+[0-9{$#-]"
"	number/variable/define/expression/label
syn match directive "^\s*%\(dw\)\ze\s\+[a-z0-9{$#.-]"
"	nothing
syn match directive "^\s*%\(else\|end\|rpe\|wpop\|wpsh\)\s*$"
"	number/variable/define/expression/label string/tostring
syn match directive "^\s*%\(assert\)\ze\s\+[a-z0-9$#.-][^ \t]*\s\+\(\"\|%tostring\)"
syn match directive "^\s*%\(assert\)\ze\s\+{.*}\s\+\(\"\|%tostring\)"
"	label/variable number/variable/define/expression
syn match directive "^\s*%\(assign\)\ze\s\+\(\$\)\?[a-z][^ \t]*\s\+[0-9{$#-]"
"	label/define   number/variable/define/expression
syn match directive "^\s*%\(define\)\ze\s\+\(#\)\?[a-z][^ \t]*\s\+[0-9{$#-]"
"	anything
syn match directive "%\(tostring\)\ze\s\+[^ \t]" 
"	anything
syn match directive "%\(typeof\)\ze\s\+[^ \t]" containedin=expression
"	anything anything
syn match directive "%\(same\)\ze\s\+[^ \t]\+\s\+[^ \t]" containedin=expression
"	define
syn match directive "%isdefined\ze\s\+#" containedin=expression
"	variable
syn match directive "%isassigned\ze\s\+\$" containedin=expression


"match all instructions by default as error, if they are correct, they are overwritten later
"excluding ones that take no argument
syn match error "\<\(add\|and\|cal\|cmp\|div\|int\)\>"
syn match error "\<\(mcs\|mcu\|mov\|mrd\|mro\|mul\)\>"
syn match error "\<\(mwo\|mwr\|not\|orr\|pop\|prd\)\>"
syn match error "\<\(psh\|pwr\|pxr\|pxw\|ret\|shl\)\>"
syn match error "\<\(shr\|sub\|tst\|xor\|xrd\|xwr\)\>"
syn match error "\<\(jcs\|jcu\|jmp\)\>"

"	reg reg/num/def/var/lab
syn match instruction "\<\(add\|and\|cmp\|div\|mov\|mrd\|mro\|mul\|not\|orr\|shl\|shr\|sub\|tst\|xor\)\>\ze\s\+R[0-7][ \t,]\+[a-zA-Z0-9$#{.-]"
"	reg/num/def/var/lab reg
syn match instruction "\<\(mwo\|mwr\)\>\ze\s\+[a-zA-Z0-9$#{.-][^ \t]*[ \t,]\+R[0-7]"
"	reg/num/def/var/lab
syn match instruction "\<\(cal\|jmp\)\>\ze\s\+[a-zA-Z0-9$#{.-]"
"	nothing
syn match instruction "\<\(hcf\|hlt\|irt\|nop\)\>"
"	ccc num/def/var/lab
syn match instruction "\<\(jcs\|jcu\)\>\s\+\<[LEGZ]\+\>\ze[ \t,]\+[0-9a-zA-Z#${.-]" contains=ccc
"	ccc R R
syn match instruction "\<\(mcs\|mcu\)\>\s\+\<[LEGZ]\+\>\ze[ \t,]\+R[0-7][ \t,]\+R[0-7]" contains=ccc
"	flags/nothing
syn match instruction "\<\(ret\)\>\(\s\+\<[SOCZE]*\>\)\?" contains=flag
" 	num/def/var
syn match instruction "\<\(int\)\>\ze\s\+[0-9$#{-]"
"	reg
syn match instruction "\<\(pop\|psh\)\>\ze\s\+R[0-7]"
"	reg reg/num/def/var
syn match instruction "\<\(prd\)\>\ze\s\+R[0-7][ \t,]\+[R0-9$#{-]"
"	reg/num/def/var reg
syn match instruction "\<\(pwr\)\>\ze\s\+[R0-9$#{-][^ \t]*[ \t,]\+R[0-7]"
"	reg num/def/var
syn match instruction "\<\(pxr\)\>\ze\s\+R[0-7][ \t,]\+[0-9$#{-]"
"	num/def/var reg
syn match instruction "\<\(pxw\)\>\ze\s\+[0-9$#{-][^ \t]*[ \t,]\+R[0-7]"
"	ext reg/num/def/var/lab
syn match instruction "\<\(xwr\)\>\ze\s\+\(IP\|UI\|SP\|LR\|FL\|F1\|F2\|OF\)[ \t,]\+[a-zA-Z0-9$#{.-]"
"	reg ext
syn match instruction "\<\(xrd\)\>\ze\s\+R[0-7][ \t,]\+\(IP\|CF\|SP\|LR\|FL\|F1\|F2\|OF\)"

"numbers
syn match number "-\?[0-9]\+" containedin=expression
syn match number "-\?0b[0-1]\+" containedin=expression
syn match number "-\?0x[0-9A-Fa-f]\+" containedin=expression

"registers
syn match  register  "R[0-7]"
syn keyword xregister IP UI CF SP LR FL F1 F2 OF
syn keyword error R8 R9

"
"
syn region comment	start="/\*" end="\*/"
syn region comment	start="//" end="$"
syn region comment	start=";" end="$"
syn region string	start="\"" end="\""
"
syn match   variable  "\$[a-zA-Z][a-zA-Z0-9_]*" containedin=expression
syn match   define    "#[a-zA-Z_][a-zA-Z0-9_]*" containedin=expression
"
syn match   macro "@[^ \t\n(]\+" containedin=expression
syn match   macro "@macro.*$"
syn match   macro "\<_[^ \t\n]\+\>"
"
"

syn match  ccc  "\<\(jcs\|jcu\|mcu\|mcs\)\s\+\zs\<[LEGZ]\+\>" contained containedin=instruction
syn match flag "\<\(ret\)\>\s\+\zs\<[SOCZE]*\>" contained containedin=instruction


let b:current_syntax = "sasm"

let &cpo = s:cpo_save
unlet s:cpo_save

hi directive   ctermfg=magenta  ctermbg=black
hi instruction ctermfg=cyan     ctermbg=black
hi variable    ctermfg=white    ctermbg=black
hi define      ctermfg=white    ctermbg=black
hi operator    ctermfg=green    ctermbg=black
hi register    ctermfg=141      ctermbg=black
hi xregister   ctermfg=141      ctermbg=black
hi label       ctermfg=brown    ctermbg=black
hi labeldef    ctermfg=brown    ctermbg=black
hi number      ctermfg=darkred  ctermbg=black
hi ccc         ctermfg=129      ctermbg=black
hi flag        ctermfg=129      ctermbg=black
hi string      ctermfg=darkred  ctermbg=black
hi comment     ctermfg=25       ctermbg=black
hi macro       ctermfg=red      ctermbg=black
hi expression  ctermfg=yellow   ctermbg=black
hi separator   ctermfg=green    ctermbg=black
hi parentheses ctermfg=green    ctermbg=black
hi error       ctermfg=white    ctermbg=red
