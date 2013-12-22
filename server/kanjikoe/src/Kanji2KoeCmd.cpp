/*-------------------------------------------------------------
	Kanji2KoeCmd - かな漢字混じりテキストを音声記号列に変換

	$ Kanji2KoeCmd (dic_dir) < in.txt > out.koe

	ビルド
	  あらかじめ
	  libAqKanji2Koe.so.X.Xを ldconfigコマンドで共有ライブラリ登録しておくこと
	  $ g++  -I./lib -o Kanji2KoeCmd samples/Kanji2KoeCmd.cpp -lAqKanji2Koe

	実行時の配置

		|- Kanji2KoeCmd
		|- aq_dic/		辞書フォルダ(aq_dic)を同じディレクトリに配置
			|- aqdic.bin
			|- aq_user.dic (ユーザ辞書:任意)
			|- CREDITS

	実行
	$ echo 音声合成テスト | ./Kanji2KoeCmd

	2011/01/14	N.Yamazaki	Creation
	2013/06/27	N.Yamazaki	Ver.2用に一部修正
    2014/11/10  S.Aoyama    出力を カナ->ローマ字 にした。

-------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>		// setlocale()
#include <aqk2k/AqKanji2Koe.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <unicode/translit.h>

#define	NSTR	4096
#define DICPATH "/aq_dic_large"

char* GetPathDic(const char* pathModule)
{
	char* p = strrchr((char*)pathModule, '/');
	if(p==NULL){
		return strdup("." DICPATH);
	}
	char* path = (char*)malloc(strlen(pathModule)+strlen(DICPATH)+1);
	strncpy(path, pathModule, p-pathModule);
	strcpy(path+(p-pathModule), DICPATH);
	return path;
}

int str2romaji(const char* str, char* dst, int lim)
{
    // Creating Transliterator instance.
    int result_len = 0;
    UErrorCode error = U_ZERO_ERROR;
    boost::shared_ptr<Transliterator> t(Transliterator::createInstance("Latin-Katakana", UTRANS_REVERSE, error));
    icu::UnicodeString src(str, "utf8");

    // translate
    t->transliterate(src);

    // getting translated data.
    result_len = src.length();
    if (result_len < 0) return -1;
    if (lim < (result_len + 1)) result_len = lim - 1;
    src.extract(0, result_len, dst, "utf8");

    return result_len;
}

template <typename T> int printbit(T x)
{
    int len = sizeof(T) * 8;
    int i;
    for(int i = 0; i < len; i++) {
        putchar(((x >> (len - (i + 1))) & 1) ? '1' : '0');
        if (((i+1) % 8) == 0) putchar(' ');
    }

    return len;
}

int replace_utf8(char* src, const char* find, const char* replacer)
{
    int i, j;
    int len = strlen(src);
    int flen = strlen(find);
    int rlen = strlen(replacer);
    if (flen < rlen) return -1;
    for (i=0; i < len - (flen - 1); i++) {
    loop_root:
        for (j=0; j < flen; j++) {
            // if missmatch then continue;
            if (src[i+j] != find[j]) { i++; if (i < len - (flen - 1)) goto loop_root; else goto loop_end; } // continue outer;
        }
        // found !!
        for (j=0; j < rlen; j++) {
            src[i+j] = replacer[j];
        }
        if (i+flen == len) { // If the found pattern was the last, just shift null-char point.
            src[i+rlen] = 0;
        } else { // or not, shift trailing data too.
            memmove(src + i + rlen, src + i + flen, len - (i + flen));
            src[len - (flen - rlen)] = 0;
        }
        len -= (flen - rlen);
    }
loop_end:
    return len;
}

int main(int ac, char **av)
{
	int iret;
	int i, j;
	char kanji[NSTR];
	char koe[NSTR];
    char koer[NSTR];
	void* hAqKanji2Koe;

	if(ac==1){
		char* pPathDic  = GetPathDic(av[0]);
		hAqKanji2Koe = AqKanji2Koe_Create(pPathDic, &iret);
		free(pPathDic);
	}
	else {
		hAqKanji2Koe = AqKanji2Koe_Create(av[1], &iret);
	}

	if(hAqKanji2Koe==0){
		fprintf(stderr, "ERR: can not initialize Dictionary(%d)\n", iret);
		fprintf(stderr, "USAGE: $ Kanji2KoeCmd (dic_dir) < in.txt > out.koe\n");
		return iret;
	}

	for(i=0; ; i++){
		if(fgets(kanji, NSTR, stdin)==0) break;
		iret = AqKanji2Koe_Convert(hAqKanji2Koe, kanji, koe, NSTR);
		if(iret!=0) {
			fprintf(stderr, "ERR: AqKanji2Koe_Convert()=%d\n", iret);
			break;
		}

		// fprintf(stdout, "original: %s\n", koe);
        int rres = replace_utf8(koe, "ー", "-");
        if (rres < 0) goto replace_utf8_failed;
        rres = replace_utf8(koe, "？", "?");
        if (rres < 0) {
        replace_utf8_failed:
            fprintf(stderr, "ERR: replace_utf8 failed.\n", iret);
            exit(1);
        }
		// fprintf(stdout, "original2: %s\n", koe);

        koer[0] = 0;
        int res = str2romaji(koe, koer, NSTR);
        // fprintf(stdout, "romaji: %d %s\n", res, koer);
        fprintf(stdout, "%s\n", koer);
	}

	AqKanji2Koe_Release(hAqKanji2Koe);
	return 0;
}
