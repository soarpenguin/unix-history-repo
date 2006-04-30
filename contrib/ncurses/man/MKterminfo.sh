#!/bin/sh
#***************************************************************************
# Copyright (c) 1998,2000,2001 Free Software Foundation, Inc.              *
#                                                                          *
# Permission is hereby granted, free of charge, to any person obtaining a  *
# copy of this software and associated documentation files (the            *
# "Software"), to deal in the Software without restriction, including      *
# without limitation the rights to use, copy, modify, merge, publish,      *
# distribute, distribute with modifications, sublicense, and/or sell       *
# copies of the Software, and to permit persons to whom the Software is    *
# furnished to do so, subject to the following conditions:                 *
#                                                                          *
# The above copyright notice and this permission notice shall be included  *
# in all copies or substantial portions of the Software.                   *
#                                                                          *
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
# IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
#                                                                          *
# Except as contained in this notice, the name(s) of the above copyright   *
# holders shall not be used in advertising or otherwise to promote the     *
# sale, use or other dealings in this Software without prior written       *
# authorization.                                                           *
#***************************************************************************
#
# $Id: MKterminfo.sh,v 1.9 2001/09/01 23:06:18 tom Exp $
#
# MKterminfo.sh -- generate terminfo.5 from Caps tabular data
#
# This script takes terminfo.head and terminfo.tail and splices in between
# them a table derived from the Caps data file.  Besides avoiding having
# the docs fall out of sync with the table, this also lets us set up tbl
# commands for better formatting of the table.
#
# NOTE: The s in this script really are control characters.  It translates
#  to \n because I couldn't get used to inserting linefeeds directly.  There
# had better be no s in the table source text.
#
# keep the order independent of locale:
LANGUAGE=C
LC_ALL=C
export LANGUAGE
export LC_ALL
#
head=$1
caps=$2
tail=$3
cat <<'EOF'
'\" t
.\" DO NOT EDIT THIS FILE BY HAND!
.\" It is generated from terminfo.head, Caps, and terminfo.tail.
.\"
.\" Note: this must be run through tbl before nroff.
.\" The magic cookie on the first line triggers this under some man programs.
EOF
cat $head

temp=temp$$
sorted=sorted$$
unsorted=unsorted$$
trap "rm -f $sorted $temp $unsorted; exit 99" 1 2 5 15

sed -n <$caps "\
/%%-STOP-HERE-%%/q
/^#%/s///p
/^#/d
s/[	]\+/	/g
s/$/T}/
s/	[A-Z0-9_()\-]\+	[0-9\-]\+	[Y\-][B\-][C\-][G\-][EK\-]\**	/	T{/
s/	bool	/	/p
s/	num	/	/p
s/	str	/	/p
" |sed -e 's/^$/../' | tr "\134" "\006" >$unsorted

rm -f $sorted
rm -f $temp
saved=no
while true
do
	data=
	read data
	test -z "$data" && break
	case "$data" in #(vi
	**) #(vi
		echo "$data" >>$temp
		saved=yes
		;;
	*)
		if test $saved = yes ; then
			saved=no
			sort $temp >>$sorted
			rm -f $temp
		fi
		echo "$data" >>$sorted
		;;
	esac
done <$unsorted
test $saved = yes && sort $temp >>$sorted

sed -e 's/^\.\.$//' $sorted | tr "\005\006" "\012\134"
cat $tail

rm -f $sorted $temp $unsorted
