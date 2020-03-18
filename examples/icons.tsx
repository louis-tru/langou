/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

import { Div, Scroll, TextNode, Hybrid, _CVD } from 'ngui';
import { Mynavpage } from './public';

const resolve = require.resolve;
const icon_str = `
\ue900
\ue901
\ue902
\ue903
\ue904
\ue905
\ue906
\ue907
\ue908
\ue909
\ue90a
\ue90b
\ue90c
\ue90d
\ue90e
\ue90f
\ue910
\ue911
\ue912
\ue913
\ue914
\ue915
\ue916
\ue917
\ue918
\ue919
\ue91a
\ue91b
\ue91c
\ue91d
\ue91e
\ue91f
\ue920
\ue921
\ue922
\ue923
\ue924
\ue925
\ue926
\ue927
\ue928
\ue929
\ue92a
\ue92b
\ue92c
\ue92d
\ue92e
\ue92f
\ue930
\ue931
\ue932
\ue933
\ue934
\ue935
\ue936
\ue937
\ue938
\ue939
\ue93a
\ue93b
\ue93c
\ue93d
\ue93e
\ue93f
\ue940
\ue941
\ue942
\ue943
\ue944
\ue945
\ue946
\ue947
\ue948
\ue949
\ue94a
\ue94b
\ue94c
\ue94d
\ue94e
\ue94f
\ue950
\ue951
\ue952
\ue953
\ue954
\ue955
\ue956
\ue957
\ue958
\ue959
\ue95a
\ue95b
\ue95c
\ue95d
\ue95e
\ue95f
\ue960
\ue961
\ue962
\ue963
\ue964
\ue965
\ue966
\ue967
\ue968
\ue969
\ue96a
\ue96b
\ue96c
\ue96d
\ue96e
\ue96f
\ue970
\ue971
\ue972
\ue973
\ue974
\ue975
\ue976
\ue977
\ue978
\ue979
\ue97a
\ue97b
\ue97c
\ue97d
\ue97e
\ue97f
\ue980
\ue981
\ue982
\ue983
\ue984
\ue985
\ue986
\ue987
\ue988
\ue989
\ue98a
\ue98b
\ue98c
\ue98d
\ue98e
\ue98f
\ue990
\ue991
\ue992
\ue993
\ue994
\ue995
\ue996
\ue997
\ue998
\ue999
\ue99a
\ue99b
\ue99c
\ue99d
\ue99e
\ue99f
\ue9a0
\ue9a1
\ue9a2
\ue9a3
\ue9a4
\ue9a5
\ue9a6
\ue9a7
\ue9a8
\ue9a9
\ue9aa
\ue9ab
\ue9ac
\ue9ad
\ue9ae
\ue9af
\ue9b0
\ue9b1
\ue9b2
\ue9b3
\ue9b4
\ue9b5
\ue9b6
\ue9b7
\ue9b8
\ue9b9
\ue9ba
\ue9bb
\ue9bc
\ue9bd
\ue9be
\ue9bf
\ue9c0
\ue9c1
\ue9c2
\ue9c3
\ue9c4
\ue9c5
\ue9c6
\ue9c7
\ue9c8
\ue9c9
\ue9ca
\ue9cb
\ue9cc
\ue9cd
\ue9ce
\ue9cf
\ue9d0
\ue9d1
\ue9d2
\ue9d3
\ue9d4
\ue9d5
\ue9d6
\ue9d7
\ue9d8
\ue9d9
\ue9da
\ue9db
\ue9dc
\ue9dd
\ue9de
\ue9df
\ue9e0
\ue9e1
\ue9e2
\ue9e3
\ue9e4
\ue9e5
\ue9e6
\ue9e7
\ue9e8
\ue9e9
\ue9ea
\ue9eb
\ue9ec
\ue9ed
\ue9ee
\ue9ef
\ue9f0
\ue9f1
\ue9f2
\ue9f3
\ue9f4
\ue9f5
\ue9f6
\ue9f7
\ue9f8
\ue9f9
\ue9fa
\ue9fb
\ue9fc
\ue9fd
\ue9fe
\ue9ff
\uea00
\uea01
\uea02
\uea03
\uea04
\uea05
\uea06
\uea07
\uea08
\uea09
\uea0a
\uea0b
\uea0c
\uea0d
\uea0e
\uea0f
\uea10
\uea11
\uea12
\uea13
\uea14
\uea15
\uea16
\uea17
\uea18
\uea19
\uea1a
\uea1b
\uea1c
\uea1d
\uea1e
\uea1f
\uea20
\uea21
\uea22
\uea23
\uea24
\uea25
\uea26
\uea27
\uea28
\uea29
\uea2a
\uea2b
\uea2c
\uea2d
\uea2e
\uea2f
\uea30
\uea31
\uea32
\uea33
\uea34
\uea35
\uea36
\uea37
\uea38
\uea39
\uea3a
\uea3b
\uea3c
\uea3d
\uea3e
\uea3f
\uea40
\uea41
\uea42
\uea43
\uea44
\uea45
\uea46
\uea47
\uea48
\uea49
\uea4a
\uea4b
\uea4c
\uea4d
\uea4e
\uea4f
\uea50
\uea51
\uea52
\uea53
\uea54
\uea55
\uea56
\uea57
\uea58
\uea59
\uea5a
\uea5b
\uea5c
\uea5d
\uea5e
\uea5f
\uea60
\uea61
\uea62
\uea63
\uea64
\uea65
\uea66
\uea67
\uea68
\uea69
\uea6a
\uea6b
\uea6c
\uea6d
\uea6e
\uea6f
\uea70
\uea71
\uea72
\uea73
\uea74
\uea75
\uea76
\uea77
\uea78
\uea79
\uea7a
\uea7b
\uea7c
\uea7d
\uea7e
\uea7f
\uea80
\uea81
\uea82
\uea83
\uea84
\uea85
\uea86
\uea87
\uea88
\uea89
\uea8a
\uea8b
\uea8c
\uea8d
\uea8e
\uea8f
\uea90
\uea91
\uea92
\uea93
\uea94
\uea95
\uea96
\uea97
\uea98
\uea99
\uea9a
\uea9b
\uea9c
\uea9d
\uea9e
\uea9f
\ueaa0
\ueaa1
\ueaa2
\ueaa3
\ueaa4
\ueaa5
\ueaa6
\ueaa7
\ueaa8
\ueaa9
\ueaaa
\ueaab
\ueaac
\ueaad
\ueaae
\ueaaf
\ueab0
\ueab1
\ueab2
\ueab3
\ueab4
\ueab5
\ueab6
\ueab7
\ueab8
\ueab9
\ueaba
\ueabb
\ueabc
\ueabd
\ueabe
\ueabf
\ueac0
\ueac1
\ueac2
\ueac3
\ueac4
\ueac5
\ueac6
\ueac7
\ueac8
\ueac9
\ueaca
\ueacb
\ueacc
\ueacd
\ueace
\ueacf
\uead0
\uead1
\uead2
\uead3
\uead4
\uead5
\uead6
\uead7
\uead8
\uead9
\ueada
\ueadb
\ueadc
\ueadd
\ueade
\ueadf
\ueae0
\ueae1
\ueae2
\ueae3
\ueae4
\ueae5
\ueae6
\ueae7
\ueae8
\ueae9
\ueaea
\ueaeb
\ueaec
\ueaed
\ueaee
\ueaef
\ueaf0
\ueaf1
\ueaf2
\ueaf3
\ueaf4
\ueaf5
\ueaf6
\ueaf7
\ueaf8
\ueaf9
\ueafa
\ueafb
\ueafc
\ueafd
\ueafe
\ueaff
\ueb00
\ueb01
\ueb02
\ueb03
\ueb04
\ueb05
\ueb06
\ueb07
\ueb08
\ueb09
\ueb0a
\ueb0b
\ueb0c
\ueb0d
\ueb0e
\ueb0f
\ueb10
\ueb11
\ueb12
\ueb13
\ueb14
\ueb15
\ueb16
\ueb17
\ueb18
\ueb19
\ueb1a
\ueb1b
\ueb1c
\ueb1d
\ueb1e
\ueb1f
\ueb20
\ueb21
\ueb22
\ueb23
\ueb24
\ueb25
\ueb26
\ueb27
\ueb28
\ueb29
\ueb2a
\ueb2b
\ueb2c
\ueb2d
\ueb2e
\ueb2f
\ueb30
\ueb31
\ueb32
\ueb33
\ueb34
\ueb35
\ueb36
\ueb37
\ueb38
\ueb39
\ueb3a
\ueb3b
\ueb3c
\ueb3d
\ueb3e
\ueb3f
\ueb40
\ueb41
\ueb42
\ueb43
\ueb44
\ueb45
\ueb46
\ueb47
\ueb48
\ueb49
\ueb4a
\ueb4b
\ueb4c
\ueb4d
\ueb4e
\ueb4f
\ueb50
\ueb51
\ueb52
\ueb53
\ueb54
\ueb55
\ueb56
\ueb57
\ueb58
\ueb59
\ueb5a
\ueb5b
\ueb5c
\ueb5d
\ueb5e
\ueb5f
\ueb60
\ueb61
\ueb62
\ueb63
\ueb64
\ueb65
\ueb66
\ueb67
\ueb68
\ueb69
\ueb6a
\ueb6b
\ueb6c
\ueb6d
\ueb6e
\ueb6f
\ueb70
\ueb71
\ueb72
\ueb73
\ueb74
\ueb75
\ueb76
\ueb77
\ueb78
\ueb79
\ueb7a
\ueb7b
\ueb7c
\ueb7d
\ueb7e
\ueb7f
\ueb80
\ueb81
\ueb82
\ueb83
\ueb84
\ueb85
\ueb86
\ueb87
\ueb88
\ueb89
\ueb8a
\ueb8b
\ueb8c
\ueb8d
\ueb8e
\ueb8f
\ueb90
\ueb91
\ueb92
\ueb93
\ueb94
\ueb95
\ueb96
\ueb97
\ueb98
\ueb99
\ueb9a
\ueb9b
\ueb9c
\ueb9d
\ueb9e
\ueb9f
\ueba0
\ueba1
\ueba2
\ueba3
\ueba4
\ueba5
\ueba6
\ueba7
\ueba8
\ueba9
\uebaa
\uebab
\uebac
\uebad
\uebae
\uebaf
\uebb0
\uebb1
\uebb2
\uebb3
\uebb4
\uebb5
\uebb6
\uebb7
\uebb8
\uebb9
\uebba
\uebbb
\uebbc
\uebbd
\uebbe
\uebbf
\uebc0
\uebc1
\uebc2
\uebc3
\uebc4
\uebc5
\uebc6
\uebc7
\uebc8
\uebc9
\uebca
\uebcb
\uebcc
\uebcd
\uebce
\uebcf
\uebd0
\uebd1
\uebd2
\uebd3
\uebd4
\uebd5
\uebd6
\uebd7
\uebd8
\uebd9
\uebda
\uebdb
\uebdc
\uebdd
\uebde
\uebdf
\uebe0
\uebe1
\uebe2
\uebe3
\uebe4
\uebe5
\uebe6
\uebe7
\uebe8
\uebe9
\uebea
\uebeb
\uebec
\uebed
\uebee
\uebef
\uebf0
\uebf1
\uebf2
\uebf3
\uebf4
\uebf5
\uebf6
\uebf7
\uebf8
\uebf9
\uebfa
\uebfb
\uebfc
\uebfd
\uebfe
\uebff
\uec00
\uec01
\uec02
\uec03
\uec04
\uec05
\uec06
\uec07
\uec08
\uec09
\uec0a
\uec0b
\uec0c
\uec0d
\uec0e
\uec0f
\uec10
\uec11
\uec12
\uec13
\uec14
\uec15
\uec16
\uec17
\uec18
\uec19
\uec1a
\uec1b
\uec1c
\uec1d
\uec1e
\uec1f
\uec20
\uec21
\uec22
\uec23
\uec24
\uec25
\uec26
\uec27
\uec28
\uec29
\uec2a
\uec2b
\uec2c
\uec2d
\uec2e
\uec2f
\uec30
\uec31
\uec32
\uec33
\uec34
\uec35
\uec36
\uec37
\uec38
\uec39
\uec3a
\uec3b
\uec3c
\uec3d
\uec3e
\uec3f
\uec40
\uec41
\uec42
\uec43
\uec44
\uec45
\uec46
\uec47
\uec48
\uec49
\uec4a
\uec4b
\uec4c
\uec4d
\uec4e
\uec4f
\uec50
\uec51
\uec52
\uec53
\uec54
\uec55
\uec56
\uec57
\uec58
\uec59
\uec5a
\uec5b
\uec5c
\uec5d
\uec5e
\uec5f
\uec60
\uec61
\uec62
\uec63
\uec64
\uec65
\uec66
\uec67
\uec68
\uec69
\uec6a
\uec6b
\uec6c
\uec6d
\uec6e
\uec6f
\uec70
\uec71
\uec72
\uec73
\uec74
\uec75
\uec76
\uec77
\uec78
\uec79
\uec7a
\uec7b
\uec7c
\uec7d
\uec7e
\uec7f
\uec80
\uec81
\uec82
\uec83
\uec84
\uec85
\uec86
\uec87
\uec88
\uec89
\uec8a
\uec8b
\uec8c
\uec8d
\uec8e
\uec8f
\uec90
\uec91
\uec92
\uec93
\uec94
\uec95
\uec96
\uec97
\uec98
\uec99
\uec9a
\uec9b
\uec9c
\uec9d
\uec9e
\uec9f
\ueca0
\ueca1
\ueca2
\ueca3
\ueca4
\ueca5
\ueca6
\ueca7
\ueca8
\ueca9
\uecaa
\uecab
\uecac
\uecad
\uecae
\uecaf
\uecb0
\uecb1
\uecb2
\uecb3
\uecb4
\uecb5
\uecb6
\uecb7
\uecb8
\uecb9
\uecba
\uecbb
\uecbc
\uecbd
\uecbe
\uecbf
\uecc0
\uecc1
\uecc2
\uecc3
\uecc4
\uecc5
\uecc6
\uecc7
\uecc8
\uecc9
\uecca
\ueccb
\ueccc
\ueccd
\uecce
\ueccf
\uecd0
\uecd1
\uecd2
\uecd3
\uecd4
\uecd5
\uecd6
\uecd7
\uecd8
\uecd9
\uecda
\uecdb
\uecdc
\uecdd
\uecde
\uecdf
\uece0
\uece1
\uece2
\uece3
\uece4
\uece5
\uece6
\uece7
\uece8
\uece9
\uecea
\ueceb
\uecec
\ueced
\uecee
\uecef
\uecf0
\uecf1
\uecf2
\uecf3
\uecf4
\uecf5
\uecf6
\uecf7
\uecf8
\uecf9
\uecfa
\uecfb
\uecfc
\uecfd
\uecfe
\uecff
\ued00
\ued01
\ued02
\ued03
\ued04
\ued05
\ued06
\ued07
\ued08
\ued09
\ued0a
\ued0b
\ued0c
\ued0d
\ued0e
\ued0f
\ued10
\ued11
\ued12
\ued13
\ued14
\ued15
\ued16
\ued17
\ued18
\ued19
\ued1a
\ued1b
\ued1c
\ued1d
\ued1e
\ued1f
\ued20
\ued21
\ued22
\ued23
\ued24
\ued25
\ued26
\ued27
\ued28
\ued29
\ued2a
\ued2b
\ued2c
\ued2d
\ued2e
\ued2f
\ued30
\ued31
\ued32
\ued33
\ued34
\ued35
\ued36
\ued37
\ued38
\ued39
\ued3a
\ued3b
\ued3c
\ued3d
\ued3e
\ued3f
\ued40
\ued41
\ued42
\ued43
\ued44
\ued45
\ued46
\ued47
\ued48
\ued49
\ued4a
\ued4b
\ued4c
\ued4d
\ued4e
\ued4f
\ued50
\ued51
\ued52
\ued53
\ued54
\ued55
\ued56
\ued57
\ued58
\ued59
\ued5a
\ued5b
\ued5c
\ued5d
\ued5e
\ued5f
\ued60
\ued61
\ued62
\ued63
\ued64
\ued65
\ued66
\ued67
\ued68
\ued69
\ued6a
\ued6b
\ued6c
\ued6d
\ued6e
\ued6f
\ued70
\ued71
\ued72
\ued73
\ued74
\ued75
\ued76
\ued77
\ued78
\ued79
\ued7a
\ued7b
\ued7c
\ued7d
\ued7e
\ued7f
\ued80
\ued81
\ued82
\ued83
\ued84
\ued85
\ued86
\ued87
\ued88
\ued89
\ued8a
\ued8b
\ued8c
\ued8d
\ued8e
\ued8f
\ued90
\ued91
\ued92
\ued93
\ued94
\ued95
\ued96
\ued97
\ued98
\ued99
\ued9a
\ued9b
\ued9c
\ued9d
\ued9e
\ued9f
\ueda0
\ueda1
\ueda2
\ueda3
\ueda4
\ueda5
\ueda6
\ueda7
\ueda8
\ueda9
\uedaa
\uedab
\uedac
\uedad
\uedae
\uedaf
\uedb0
\uedb1
\uedb2
\uedb3
\uedb4
\uedb5
\uedb6
\uedb7
\uedb8
\uedb9
\uedba
\uedbb
\uedbc
\uedbd
\uedbe
\uedbf
\uedc0
\uedc1
\uedc2
\uedc3
\uedc4
\uedc5
\uedc6
\uedc7
\uedc8
\uedc9
\uedca
\uedcb
\uedcc
\uedcd
\uedce
\uedcf
\uedd0
\uedd1
\uedd2
\uedd3
\uedd4
\uedd5
\uedd6
\uedd7
\uedd8
\uedd9
\uedda
\ueddb
\ueddc
\ueddd
\uedde
\ueddf
\uede0
\uede1
\uede2
\uede3
\uede4
\uede5
\uede6
\uede7
\uede8
\uede9
\uedea
\uedeb
\uedec
\ueded
\uedee
\uedef
\uedf0
\uedf1
\uedf2
\uedf3
\uedf4
\uedf5
\uedf6
\uedf7
\uedf8
\uedf9
\uedfa
\uedfb
\uedfc
\uedfd
\uedfe
\uedff
\uee00
\uee01
\uee02
\uee03
\uee04
\uee05
\uee06
\uee07
\uee08
\uee09
\uee0a
\uee0b
\uee0c
\uee0d
\uee0e
\uee0f
\uee10
\uee11
\uee12
\uee13
\uee14
\uee15
\uee16
\uee17
\uee18
\uee19
\uee1a
\uee1b
\uee1c
\uee1d
\uee1e
\uee1f
\uee20
\uee21
\uee22
\uee23
\uee24
\uee25
\uee26
\uee27
\uee28
\uee29
\uee2a
\uee2b
\uee2c
\uee2d
\uee2e
\uee2f
\uee30
\uee31
\uee32
\uee33
\uee34
\uee35
\uee36
\uee37
\uee38
\uee39
\uee3a
\uee3b
\uee3c
\uee3d
\uee3e
\uee3f
\uee40
\uee41
\uee42
\uee43
\uee44
\uee45
\uee46
\uee47
\uee48
\uee49
\uee4a
\uee4b
\uee4c
\uee4d
\uee4e
\uee4f
\uee50
\uee51
\uee52
\uee53
\uee54
\uee55
\uee56
\uee57
\uee58
\uee59
\uee5a
\uee5b
\uee5c
\uee5d
\uee5e
\uee5f
\uee60
\uee61
\uee62
\uee63
\uee64
\uee65
\uee66
\uee67
\uee68
\uee69
\uee6a
\uee6b
\uee6c
\uee6d
\uee6e
\uee6f
\uee70
\uee71
\uee72
\uee73
\uee74
\uee75
\uee76
\uee77
\uee78
\uee79
\uee7a
\uee7b
\uee7c
\uee7d
\uee7e
\uee7f
\uee80
\uee81
\uee82
\uee83
\uee84
\uee85
\uee86
\uee87
\uee88
\uee89
\uee8a
\uee8b
\uee8c
\uee8d
\uee8e
\uee8f
\uee90
\uee91
\uee92
\uee93
\uee94
\uee95
\uee96
\uee97
\uee98
\uee99
\uee9a
\uee9b
\uee9c
\uee9d
\uee9e
\uee9f
\ueea0
\ueea1
\ueea2
\ueea3
\ueea4
\ueea5
\ueea6
\ueea7
\ueea8
\ueea9
\ueeaa
\ueeab
\ueeac
\ueead
\ueeae
\ueeaf
\ueeb0
\ueeb1
\ueeb2
\ueeb3
\ueeb4
\ueeb5
\ueeb6
\ueeb7
\ueeb8
\ueeb9
\ueeba
\ueebb
\ueebc
\ueebd
\ueebe
\ueebf
\ueec0
\ueec1
\ueec2
\ueec3
\ueec4
\ueec5
\ueec6
\ueec7
\ueec8
\ueec9
\ueeca
\ueecb
\ueecc
\ueecd
\ueece
\ueecf
\ueed0
\ueed1
\ueed2
\ueed3
\ueed4
\ueed5
\ueed6
\ueed7
\ueed8
\ueed9
\ueeda
\ueedb
\ueedc
\ueedd
\ueede
\ueedf
\ueee0
\ueee1
\ueee2
\ueee3
\ueee4
\ueee5
\ueee6
\ueee7
\ueee8
\ueee9
\ueeea
\ueeeb
\ueeec
\ueeed
\ueeee
\ueeef
\ueef0
\ueef1
\ueef2
\ueef3
\ueef4
\ueef5
\ueef6
\ueef7
\ueef8
\ueef9
\ueefa
\ueefb
\ueefc
\ueefd
\ueefe
\ueeff
\uef00
\uef01
\uef02
\uef03
\uef04
\uef05
\uef06
\uef07
\uef08
\uef09
\uef0a
\uef0b
\uef0c
\uef0d
\uef0e
\uef0f
\uef10
\uef11
\uef12
\uef13
\uef14
\uef15
\uef16
\uef17
\uef18
\uef19
\uef1a
\uef1b
\uef1c
\uef1d
\uef1e
\uef1f
\uef20
\uef21
\uef22
\uef23
\uef24
\uef25
\uef26
\uef27
\uef28
\uef29
\uef2a
\uef2b
\uef2c
\uef2d
\uef2e
\uef2f
\uef30
\uef31
\uef32
\uef33
\uef34
\uef35
\uef36
\uef37
\uef38
\uef39
\uef3a
\uef3b
\uef3c
\uef3d
\uef3e
\uef3f
\uef40
\uef41
\uef42
\uef43
\uef44
\uef45
\uef46
\uef47
\uef48
\uef49
`;

class Controller extends Mynavpage {
	source = resolve(__filename);
	render() {
		return super.render(
			<Scroll width="full" height="full" bounceLock={0}>
				<Div margin={10} width="full">
					{
						Array.from({length: icon_str.length}).map((e,i)=>{
							var unicode = icon_str.charCodeAt(i);
							if (unicode > 255) {
								return (
									<Hybrid marginBottom={10} textAlign="center" width="25%">
										<TextNode textSize={28} textFamily="icomoon-ultimate" value={icon_str[i]} />
										<TextNode textColor="#555" value={` ${unicode.toString(16)}`} />
									</Hybrid>
								);
							}
						})
					}
				</Div>
			</Scroll>
		);
	}
}

export default ()=>{
	<Controller title="Icons" />
}