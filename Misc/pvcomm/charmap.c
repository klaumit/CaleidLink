const unsigned char charmap_pv2ansi[] = {
    0, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 10, 32, 32, 10, 32, 32, 
    32, 32, 32, 32, 32, 32, 32, 32, 
    32, 32, 32, 32, 32, 32, 32, 32,
    32 /* */, 33 /*!*/, 34 /*"*/, 35 /*#*/, 36 /*$*/, 37 /*%*/, 38 /*&*/, 39 /*'*/, 
    40 /*(*/, 41 /*)*/, 42 /***/, 43 /*+*/, 44 /*,*/, 45 /*-*/, 46 /*.*/, 47 /*/*/, 
    48 /*0*/, 49 /*1*/, 50 /*2*/, 51 /*3*/, 52 /*4*/, 53 /*5*/, 54 /*6*/, 55 /*7*/, 
    56 /*8*/, 57 /*9*/, 58 /*:*/, 59 /*;*/, 60 /*<*/, 61 /*=*/, 62 /*>*/, 63 /*?*/, 
    64 /*@*/, 65 /*A*/, 66 /*B*/, 67 /*C*/, 68 /*D*/, 69 /*E*/, 70 /*F*/, 71 /*G*/, 
    72 /*H*/, 73 /*I*/, 74 /*J*/, 75 /*K*/, 76 /*L*/, 77 /*M*/, 78 /*N*/, 79 /*O*/, 
    80 /*P*/, 81 /*Q*/, 82 /*R*/, 83 /*S*/, 84 /*T*/, 85 /*U*/, 86 /*V*/, 87 /*W*/, 
    88 /*X*/, 89 /*Y*/, 90 /*Z*/, 91 /*[*/, 92 /*\*/, 93 /*]*/, 94 /*^*/, 95 /*_*/, 
    96 /*`*/, 97 /*a*/, 98 /*b*/, 99 /*c*/, 100 /*d*/, 101 /*e*/, 102 /*f*/, 103 /*g*/, 
    104 /*h*/, 105 /*i*/, 106 /*j*/, 107 /*k*/, 108 /*l*/, 109 /*m*/, 110 /*n*/, 111 /*o*/, 
    112 /*p*/, 113 /*q*/, 114 /*r*/, 115 /*s*/, 116 /*t*/, 117 /*u*/, 118 /*v*/, 119 /*w*/, 
    120 /*x*/, 121 /*y*/, 122 /*z*/, 123 /*{*/, 166 /*�*/, 125 /*}*/, 126 /*~*/, 32 /* */, 
    193 /*�*/, 201 /*�*/, 205 /*�*/, 211 /*�*/, 218 /*�*/, 192 /*�*/, 200 /*�*/, 204 /*�*/, 
    210 /*�*/, 217 /*�*/, 194 /*�*/, 202 /*�*/, 206 /*�*/, 212 /*�*/, 219 /*�*/, 161 /*�*/, 
    225 /*�*/, 233 /*�*/, 237 /*�*/, 243 /*�*/, 250 /*�*/, 224 /*�*/, 232 /*�*/, 236 /*�*/, 
    242 /*�*/, 249 /*�*/, 226 /*�*/, 234 /*�*/, 238 /*�*/, 244 /*�*/, 251 /*�*/, 191 /*�*/, 
    196 /*�*/, 203 /*�*/, 207 /*�*/, 214 /*�*/, 220 /*�*/, 195 /*�*/, 213 /*�*/, 209 /*�*/, 
    32 /* */, 198 /*�*/, 199 /*�*/, 197 /*�*/, 216 /*�*/, 223 /*�*/, 182 /*�*/, 162 /*�*/, 
    228 /*�*/, 235 /*�*/, 239 /*�*/, 246 /*�*/, 252 /*�*/, 227 /*�*/, 245 /*�*/, 241 /*�*/, 
    32 /* */, 230 /*�*/, 231 /*�*/, 229 /*�*/, 248 /*�*/, 163 /*�*/, 165 /*�*/, 32 /* */, 
    170 /*�*/, 186 /*�*/, 215 /*�*/, 247 /*�*/, 177 /*�*/, 176 /*�*/, 178 /*�*/, 179 /*�*/, 
    181 /*�*/, 189 /*�*/, 188 /*�*/, 190 /*�*/, 131 /*�*/, 124 /*|*/, 32 /* */, 32 /* */, 
    32 /* */, 32 /* */, 32 /* */, 167 /*�*/, 32 /* */, 32 /* */, 32 /* */, 173 /*�*/, 
    255 /*�*/, 174 /*�*/, 171 /*�*/, 187 /*�*/, 169 /*�*/, 164 /*�*/, 240 /*�*/, 208 /*�*/, 
    32 /* */, 254 /*�*/, 222 /*�*/, 253 /*�*/, 221 /*�*/, 183 /*�*/, 185 /*�*/, 128 /*�*/, 
    32 /* */, 32 /* */, 32 /* */, 32 /* */, 32 /* */, 32 /* */, 32 /* */, 32 /* */, 
    32 /* */, 32 /* */, 32 /* */, 32 /* */, 32 /* */, 32 /* */, 32 /* */, 32 /* */, 
    32 /* */, 32 /* */, 32 /* */, 32 /* */, 32 /* */, 32 /* */, 32 /* */, 32 /* */, 
    };

const unsigned char charmap_ansi2pv[] = {
    0, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 13, 32, 32, 13, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32,
    32 /* */, 33 /*!*/, 34 /*"*/, 35 /*#*/, 36 /*$*/, 37 /*%*/, 38 /*&*/, 39 /*'*/,
    40 /*(*/, 41 /*)*/, 42 /***/, 43 /*+*/, 44 /*,*/, 45 /*-*/, 46 /*.*/, 47 /*/*/, 
    48 /*0*/, 49 /*1*/, 50 /*2*/, 51 /*3*/, 52 /*4*/, 53 /*5*/, 54 /*6*/, 55 /*7*/, 
    56 /*8*/, 57 /*9*/, 58 /*:*/, 59 /*;*/, 60 /*<*/, 61 /*=*/, 62 /*>*/, 63 /*?*/, 
    64 /*@*/, 65 /*A*/, 66 /*B*/, 67 /*C*/, 68 /*D*/, 69 /*E*/, 70 /*F*/, 71 /*G*/, 
    72 /*H*/, 73 /*I*/, 74 /*J*/, 75 /*K*/, 76 /*L*/, 77 /*M*/, 78 /*N*/, 79 /*O*/, 
    80 /*P*/, 81 /*Q*/, 82 /*R*/, 83 /*S*/, 84 /*T*/, 85 /*U*/, 86 /*V*/, 87 /*W*/, 
    88 /*X*/, 89 /*Y*/, 90 /*Z*/, 91 /*[*/, 92 /*\*/, 93 /*]*/, 94 /*^*/, 95 /*_*/, 
    96 /*`*/, 97 /*a*/, 98 /*b*/, 99 /*c*/, 100 /*d*/, 101 /*e*/, 102 /*f*/, 103 /*g*/, 
    104 /*h*/, 105 /*i*/, 106 /*j*/, 107 /*k*/, 108 /*l*/, 109 /*m*/, 110 /*n*/, 111 /*o*/, 
    112 /*p*/, 113 /*q*/, 114 /*r*/, 115 /*s*/, 116 /*t*/, 117 /*u*/, 118 /*v*/, 119 /*w*/, 
    120 /*x*/, 121 /*y*/, 122 /*z*/, 123 /*{*/, 205 /*|*/, 125 /*}*/, 126 /*~*/, 32 /**/, 
    231 /*�*/, 32 /*�*/, 32 /*�*/, 204 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 
    32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 
    32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 
    32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 32 /*�*/, 
    32 /*�*/, 143 /*�*/, 175 /*�*/, 189 /*�*/, 221 /*�*/, 190 /*�*/, 124 /*�*/, 211 /*�*/, 
    32 /*�*/, 220 /*�*/, 192 /*�*/, 218 /*�*/, 32 /*�*/, 215 /*�*/, 217 /*�*/, 32 /*�*/, 
    197 /*�*/, 196 /*�*/, 198 /*�*/, 199 /*�*/, 32 /*�*/, 200 /*�*/, 174 /*�*/, 229 /*�*/, 
    32 /*�*/, 230 /*�*/, 193 /*�*/, 219 /*�*/, 202 /*�*/, 201 /*�*/, 203 /*�*/, 159 /*�*/, 
    133 /*�*/, 128 /*�*/, 138 /*�*/, 165 /*�*/, 160 /*�*/, 171 /*�*/, 169 /*�*/, 170 /*�*/, 
    134 /*�*/, 129 /*�*/, 139 /*�*/, 161 /*�*/, 135 /*�*/, 130 /*�*/, 140 /*�*/, 162 /*�*/, 
    223 /*�*/, 167 /*�*/, 136 /*�*/, 131 /*�*/, 141 /*�*/, 166 /*�*/, 163 /*�*/, 194 /*�*/, 
    172 /*�*/, 137 /*�*/, 132 /*�*/, 142 /*�*/, 164 /*�*/, 228 /*�*/, 226 /*�*/, 173 /*�*/, 
    149 /*�*/, 144 /*�*/, 154 /*�*/, 181 /*�*/, 176 /*�*/, 187 /*�*/, 185 /*�*/, 186 /*�*/, 
    150 /*�*/, 145 /*�*/, 155 /*�*/, 177 /*�*/, 151 /*�*/, 146 /*�*/, 156 /*�*/, 178 /*�*/, 
    222 /*�*/, 183 /*�*/, 152 /*�*/, 147 /*�*/, 157 /*�*/, 182 /*�*/, 179 /*�*/, 195 /*�*/, 
    188 /*�*/, 153 /*�*/, 148 /*�*/, 158 /*�*/, 180 /*�*/, 227 /*�*/, 225 /*�*/, 216 /*�*/, 
    };
