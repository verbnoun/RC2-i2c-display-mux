//============================================================================
// Japanese Character Mapping System Implementation
// Maps UTF-8 Japanese characters to font array indices
//============================================================================

#include "japanese_char_map.h"
#include "ssd1306_graphics.h"

//============================================================================
// CHARACTER MAPPING TABLE
//============================================================================

// UTF-8 to font array index mapping
// Based on font2c character order: ASCII → Japanese punctuation → Hiragana → Katakana
typedef struct {
    uint32_t utf8_code;  // UTF-8 character code
    uint16_t font_index; // Index in font array
} char_mapping_t;

// Character mapping table - sorted by Unicode value for binary search
// Generated from font2c output using generate_char_mapping.py
static const char_mapping_t char_map[] = {
    {0x0020,   0},  //  
    {0x0021,   1},  // !
    {0x0022,   2},  // "
    {0x0023,   3},  // #
    {0x0024,   4},  // $
    {0x0025,   5},  // %
    {0x0026,   6},  // &
    {0x0027,   7},  // '
    {0x0028,   8},  // (
    {0x0029,   9},  // )
    {0x002A,  10},  // *
    {0x002B,  11},  // +
    {0x002C,  12},  // ,
    {0x002D,  13},  // -
    {0x002E,  14},  // .
    {0x002F,  15},  // /
    {0x0030,  16},  // 0
    {0x0031,  17},  // 1
    {0x0032,  18},  // 2
    {0x0033,  19},  // 3
    {0x0034,  20},  // 4
    {0x0035,  21},  // 5
    {0x0036,  22},  // 6
    {0x0037,  23},  // 7
    {0x0038,  24},  // 8
    {0x0039,  25},  // 9
    {0x003A,  26},  // :
    {0x003B,  27},  // ;
    {0x003C,  28},  // <
    {0x003D,  29},  // =
    {0x003E,  30},  // >
    {0x003F,  31},  // ?
    {0x0040,  32},  // @
    {0x0041,  33},  // A
    {0x0042,  34},  // B
    {0x0043,  35},  // C
    {0x0044,  36},  // D
    {0x0045,  37},  // E
    {0x0046,  38},  // F
    {0x0047,  39},  // G
    {0x0048,  40},  // H
    {0x0049,  41},  // I
    {0x004A,  42},  // J
    {0x004B,  43},  // K
    {0x004C,  44},  // L
    {0x004D,  45},  // M
    {0x004E,  46},  // N
    {0x004F,  47},  // O
    {0x0050,  48},  // P
    {0x0051,  49},  // Q
    {0x0052,  50},  // R
    {0x0053,  51},  // S
    {0x0054,  52},  // T
    {0x0055,  53},  // U
    {0x0056,  54},  // V
    {0x0057,  55},  // W
    {0x0058,  56},  // X
    {0x0059,  57},  // Y
    {0x005A,  58},  // Z
    {0x005B,  59},  // [
    {0x005C,  60},  // \
    {0x005D,  61},  // ]
    {0x005E,  62},  // ^
    {0x005F,  63},  // _
    {0x0060,  64},  // `
    {0x0061,  65},  // a
    {0x0062,  66},  // b
    {0x0063,  67},  // c
    {0x0064,  68},  // d
    {0x0065,  69},  // e
    {0x0066,  70},  // f
    {0x0067,  71},  // g
    {0x0068,  72},  // h
    {0x0069,  73},  // i
    {0x006A,  74},  // j
    {0x006B,  75},  // k
    {0x006C,  76},  // l
    {0x006D,  77},  // m
    {0x006E,  78},  // n
    {0x006F,  79},  // o
    {0x0070,  80},  // p
    {0x0071,  81},  // q
    {0x0072,  82},  // r
    {0x0073,  83},  // s
    {0x0074,  84},  // t
    {0x0075,  85},  // u
    {0x0076,  86},  // v
    {0x0077,  87},  // w
    {0x0078,  88},  // x
    {0x0079,  89},  // y
    {0x007A,  90},  // z
    {0x007B,  91},  // {
    {0x007C,  92},  // |
    {0x007D,  93},  // }
    {0x007E,  94},  // ~
    {0x3001,  95},  // 、
    {0x3002,  96},  // 。
    {0x300C,  97},  // 「
    {0x300D,  98},  // 」
    {0x3010,  99},  // 【
    {0x3011, 100},  // 】
    {0x3042, 101},  // あ
    {0x3044, 102},  // い
    {0x3046, 103},  // う
    {0x3048, 104},  // え
    {0x304A, 105},  // お
    {0x304B, 106},  // か
    {0x304C, 107},  // が
    {0x304D, 108},  // き
    {0x304E, 109},  // ぎ
    {0x304F, 110},  // く
    {0x3050, 111},  // ぐ
    {0x3051, 112},  // け
    {0x3052, 113},  // げ
    {0x3053, 114},  // こ
    {0x3054, 115},  // ご
    {0x3055, 116},  // さ
    {0x3056, 117},  // ざ
    {0x3057, 118},  // し
    {0x3058, 119},  // じ
    {0x3059, 120},  // す
    {0x305A, 121},  // ず
    {0x305B, 122},  // せ
    {0x305C, 123},  // ぜ
    {0x305D, 124},  // そ
    {0x305E, 125},  // ぞ
    {0x305F, 126},  // た
    {0x3060, 127},  // だ
    {0x3061, 128},  // ち
    {0x3062, 129},  // ぢ
    {0x3064, 130},  // つ
    {0x3065, 131},  // づ
    {0x3066, 132},  // て
    {0x3067, 133},  // で
    {0x3068, 134},  // と
    {0x3069, 135},  // ど
    {0x306A, 136},  // な
    {0x306B, 137},  // に
    {0x306C, 138},  // ぬ
    {0x306D, 139},  // ね
    {0x306E, 140},  // の
    {0x306F, 141},  // は
    {0x3070, 142},  // ば
    {0x3071, 143},  // ぱ
    {0x3072, 144},  // ひ
    {0x3073, 145},  // び
    {0x3074, 146},  // ぴ
    {0x3075, 147},  // ふ
    {0x3076, 148},  // ぶ
    {0x3077, 149},  // ぷ
    {0x3078, 150},  // へ
    {0x3079, 151},  // べ
    {0x307A, 152},  // ぺ
    {0x307B, 153},  // ほ
    {0x307C, 154},  // ぼ
    {0x307D, 155},  // ぽ
    {0x307E, 156},  // ま
    {0x307F, 157},  // み
    {0x3080, 158},  // む
    {0x3081, 159},  // め
    {0x3082, 160},  // も
    {0x3083, 161},  // ゃ
    {0x3084, 162},  // や
    {0x3085, 163},  // ゅ
    {0x3086, 164},  // ゆ
    {0x3087, 165},  // ょ
    {0x3088, 166},  // よ
    {0x3089, 167},  // ら
    {0x308A, 168},  // り
    {0x308B, 169},  // る
    {0x308C, 170},  // れ
    {0x308D, 171},  // ろ
    {0x308F, 172},  // わ
    {0x3092, 173},  // を
    {0x3093, 174},  // ん
    {0x30A2, 175},  // ア
    {0x30A4, 176},  // イ
    {0x30A6, 177},  // ウ
    {0x30A8, 178},  // エ
    {0x30AA, 179},  // オ
    {0x30AB, 180},  // カ
    {0x30AC, 181},  // ガ
    {0x30AD, 182},  // キ
    {0x30AE, 183},  // ギ
    {0x30AF, 184},  // ク
    {0x30B0, 185},  // グ
    {0x30B1, 186},  // ケ
    {0x30B2, 187},  // ゲ
    {0x30B3, 188},  // コ
    {0x30B4, 189},  // ゴ
    {0x30B5, 190},  // サ
    {0x30B6, 191},  // ザ
    {0x30B7, 192},  // シ
    {0x30B8, 193},  // ジ
    {0x30B9, 194},  // ス
    {0x30BA, 195},  // ズ
    {0x30BB, 196},  // セ
    {0x30BC, 197},  // ゼ
    {0x30BD, 198},  // ソ
    {0x30BE, 199},  // ゾ
    {0x30BF, 200},  // タ
    {0x30C0, 201},  // ダ
    {0x30C1, 202},  // チ
    {0x30C2, 203},  // ヂ
    {0x30C3, 204},  // ッ
    {0x30C4, 205},  // ツ
    {0x30C5, 206},  // ヅ
    {0x30C6, 207},  // テ
    {0x30C7, 208},  // デ
    {0x30C8, 209},  // ト
    {0x30C9, 210},  // ド
    {0x30CA, 211},  // ナ
    {0x30CB, 212},  // ニ
    {0x30CC, 213},  // ヌ
    {0x30CD, 214},  // ネ
    {0x30CE, 215},  // ノ
    {0x30CF, 216},  // ハ
    {0x30D0, 217},  // バ
    {0x30D1, 218},  // パ
    {0x30D2, 219},  // ヒ
    {0x30D3, 220},  // ビ
    {0x30D4, 221},  // ピ
    {0x30D5, 222},  // フ
    {0x30D6, 223},  // ブ
    {0x30D7, 224},  // プ
    {0x30D8, 225},  // ヘ
    {0x30D9, 226},  // ベ
    {0x30DA, 227},  // ペ
    {0x30DB, 228},  // ホ
    {0x30DC, 229},  // ボ
    {0x30DD, 230},  // ポ
    {0x30DE, 231},  // マ
    {0x30DF, 232},  // ミ
    {0x30E0, 233},  // ム
    {0x30E1, 234},  // メ
    {0x30E2, 235},  // モ
    {0x30E4, 236},  // ヤ
    {0x30E6, 237},  // ユ
    {0x30E8, 238},  // ヨ
    {0x30E9, 239},  // ラ
    {0x30EA, 240},  // リ
    {0x30EB, 241},  // ル
    {0x30EC, 242},  // レ
    {0x30ED, 243},  // ロ
    {0x30EF, 244},  // ワ
    {0x30F2, 245},  // ヲ
    {0x30F3, 246},  // ン
    {0x30FB, 247},  // ・
    {0x30FC, 248},  // ー
};

#define CHAR_MAP_SIZE 249

//============================================================================
// UTF-8 DECODING FUNCTIONS
//============================================================================

/**
 * Decode UTF-8 character to Unicode codepoint
 * @param utf8_char Pointer to UTF-8 bytes
 * @param bytes_consumed Pointer to store bytes consumed
 * @return Unicode codepoint
 */
static uint32_t utf8_decode(const uint8_t* utf8_char, uint8_t* bytes_consumed) {
    if (!utf8_char || !bytes_consumed) return 0;
    
    uint32_t codepoint = 0;
    
    if (utf8_char[0] <= 0x7F) {
        // 1-byte ASCII character
        codepoint = utf8_char[0];
        *bytes_consumed = 1;
    } else if ((utf8_char[0] & 0xE0) == 0xC0) {
        // 2-byte character
        codepoint = ((utf8_char[0] & 0x1F) << 6) | (utf8_char[1] & 0x3F);
        *bytes_consumed = 2;
    } else if ((utf8_char[0] & 0xF0) == 0xE0) {
        // 3-byte character (most Japanese characters)
        codepoint = ((utf8_char[0] & 0x0F) << 12) | 
                   ((utf8_char[1] & 0x3F) << 6) | 
                   (utf8_char[2] & 0x3F);
        *bytes_consumed = 3;
    } else if ((utf8_char[0] & 0xF8) == 0xF0) {
        // 4-byte character
        codepoint = ((utf8_char[0] & 0x07) << 18) | 
                   ((utf8_char[1] & 0x3F) << 12) | 
                   ((utf8_char[2] & 0x3F) << 6) | 
                   (utf8_char[3] & 0x3F);
        *bytes_consumed = 4;
    } else {
        // Invalid UTF-8
        *bytes_consumed = 1;
        return 0;
    }
    
    return codepoint;
}

//============================================================================
// PUBLIC FUNCTIONS
//============================================================================

uint16_t japanese_char_to_index(const uint8_t* utf8_char, uint8_t* bytes_consumed) {
    if (!utf8_char || !bytes_consumed) return 0xFFFF;
    
    // Decode UTF-8 to Unicode codepoint
    uint32_t codepoint = utf8_decode(utf8_char, bytes_consumed);
    if (codepoint == 0) return 0xFFFF;
    
    // Binary search in mapping table
    int left = 0;
    int right = CHAR_MAP_SIZE - 1;
    
    while (left <= right) {
        int mid = (left + right) / 2;
        
        if (char_map[mid].utf8_code == codepoint) {
            return char_map[mid].font_index;
        } else if (char_map[mid].utf8_code < codepoint) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return 0xFFFF; // Character not found
}

uint8_t japanese_char_width(const uint8_t* utf8_char, const void* font) {
    if (!utf8_char || !font) return 0;
    
    const ssd1306_font_t* f = (const ssd1306_font_t*)font;
    
    // Get Unicode codepoint to determine character type
    uint8_t bytes_consumed;
    uint32_t codepoint = utf8_decode(utf8_char, &bytes_consumed);
    
    // ASCII characters (space to ~) use half width + 1 for better spacing
    if (codepoint >= 0x0020 && codepoint <= 0x007E) {
        return (f->width / 2) + 1;  // Half width + 1 pixel for readability
    }
    
    // Japanese characters use full width
    return f->width;
}

bool japanese_char_supported(const uint8_t* utf8_char) {
    if (!utf8_char) return false;
    
    uint8_t bytes_consumed;
    uint16_t index = japanese_char_to_index(utf8_char, &bytes_consumed);
    return (index != 0xFFFF);
}