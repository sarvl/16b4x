" Vim syntax file
" Language:		sasm
" Maintainer:		sarvel
" Contributors:		sarvel
" Last Change:		2024 Aug 28

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
syn match error "\<\(add\|and\|ann\|cal\|cmp\|crd\)\>"
syn match error "\<\(cwr\|dvu\|dvs\|fls\|int\|xor\)\>"
syn match error "\<\(jmp\|mls\|mlu\|mov\|mrd\|xrd\)\>" 
syn match error "\<\(mwr\|neg\|not\|orr\|pop\|xwr\)\>"
syn match error "\<\(shl\|shr\|srd\|sub\|swr\|tst\)\>"
syn match error "\<\(prd\|prf\|psh\|pwr\|rng\)\>" 
"conditionals
syn match error "\<[jsm]\(aa\|ae\|az\|bb\|be\|bz\|cc\|ee\|ge\|gg\|gz\)"
syn match error "\<[jsm]\(le\|ll\|lz\|nc\|ne\|no\|ns\|nz\|oo\|ss\|zz\)"


"	reg reg/num/def/var/lab
syn match instruction "\<\(add\|and\|ann\|cmp\|mls\|mlu\|mov\|mrd\|orr\|shl\|shr\|sub\|srd\|tst\|xor\)\>\ze\s\+R[0-7][ \t,]\+[a-zA-Z0-9$#{.-]"
"	reg/num/def/var/lab reg
syn match instruction "\<\(swr\|mwr\)\>\ze\s\+[a-zA-Z0-9$#{.-][^ \t]*[ \t,]\+R[0-7]"
"	reg/num/def/var/lab
syn match instruction "\<\(cal\|fls\|prf\)\>\ze\s\+[a-zA-Z0-9$#{.-]"
"	nothing
syn match instruction "\<\(hcf\|hlt\|irt\|nop\|ret\)\>"
"	num/def/var/lab
syn match instruction "\<\j\(aa\|ae\|az\|bb\|be\|bz\|cc\|ee\|ge\|gg\|gz\)\ze[ \t,]\+[0-9a-zA-Z#${.-]"
syn match instruction "\<\j\(le\|ll\|lz\|nc\|ne\|no\|ns\|nz\|oo\|ss\|zz\)\ze[ \t,]\+[0-9a-zA-Z#${.-]"
syn match instruction "\<jmp\>\ze\s\+[a-zA-Z0-9$#{.-]"
"	R R
syn match instruction "\<m\(aa\|ae\|az\|bb\|be\|bz\|cc\|ee\|ge\|gg\|gz\)\ze\([ \t,]\+R[0-7]\)\{2\}"
syn match instruction "\<m\(le\|ll\|lz\|nc\|ne\|no\|ns\|nz\|oo\|ss\|zz\)\ze\([ \t,]\+R[0-7]\)\{2\}"
syn match instruction "\<\(dvs\|dvu\)\ze\([ \t,]\+R[0-7]\)\{2\}"
" 	num/def/var
syn match instruction "\<\(int\)\>\ze\s\+[0-9$#{-]"
"	reg
syn match instruction "\<\(pop\|psh\|neg\|rng\|not\)\>\ze\s\+R[0-7]"
syn match instruction "\<s\(aa\|ae\|az\|bb\|be\|bz\|cc\|ee\|ge\|gg\|gz\)\ze[ \t,]\+R[0-7]"
syn match instruction "\<s\(le\|ll\|lz\|nc\|ne\|no\|ns\|nz\|oo\|ss\|zz\)\ze[ \t,]\+R[0-7]"
"	reg reg/num/def/var
syn match instruction "\<\(prd\|crd\)\>\ze\s\+R[0-7][ \t,]\+[R0-9$#{-]"
"	reg/num/def/var reg
syn match instruction "\<\(pwr\|cwr\)\>\ze\s\+[R0-9$#{-][^ \t]*[ \t,]\+R[0-7]"
"	ext reg/num/def/var/lab
syn match instruction "\<\(xwr\)\>\ze\s\+\(IP\|UI\|SP\|FL\)[ \t,]\+[a-zA-Z0-9$#{.-]"
"	reg ext
syn match instruction "\<\(xrd\)\>\ze\s\+R[0-7][ \t,]\+\(IP\|SP\|FL\)"

"numbers
syn match number "-\?[0-9]\+" containedin=expression
syn match number "-\?0[bB][0-1]\+" containedin=expression
syn match number "-\?0[xX][0-9A-Fa-f]\+" containedin=expression

"registers
syn match  register  "R[0-7]"
syn keyword xregister IP UI SP FL
syn keyword error R8 R9

"
"
syn region comment	start="/\*" end="\*/"
syn region comment	start="//" end="$"
syn region comment	start=";" end="$"
syn region string	start="\"" end="\""
"
syn match   variable  "\$[a-zA-Z][a-zA-Z0-9_]*" containedin=expression
syn match   define    "#[a-zA-Z][a-zA-Z0-9_]*" containedin=expression
"
syn match   macro "@[^ \t\n(]\+" containedin=expression
syn match   macro "@macro.*$"
syn match   macro "\<_[^ \t\n]\+\>"
"
"

"uncomment for special highlight  of condition codes
"syn match ccc "\<[jsm]\zs\(aa\|ae\|az\|bb\|be\|bz\|cc\|ee\|ge\|gg\|gz\)" contained containedin=instruction
"syn match ccc "\<[jsm]\zs\(le\|ll\|lz\|nc\|ne\|no\|ns\|nz\|oo\|ss\|zz\)" contained containedin=instruction


let b:current_syntax = "sasm"

let &cpo = s:cpo_save
unlet s:cpo_save

hi directive   ctermfg=magenta  
hi instruction ctermfg=cyan     
hi variable    ctermfg=white    
hi define      ctermfg=white    
hi operator    ctermfg=green    
hi register    ctermfg=141      
hi xregister   ctermfg=141      
hi label       ctermfg=brown    
hi labeldef    ctermfg=brown    
hi number      ctermfg=darkred  
hi ccc         ctermfg=129      
hi string      ctermfg=darkred  
hi comment     ctermfg=25       
hi macro       ctermfg=red      
hi expression  ctermfg=yellow   
hi separator   ctermfg=green    
hi parentheses ctermfg=green    
hi error       ctermfg=white    ctermbg=red
