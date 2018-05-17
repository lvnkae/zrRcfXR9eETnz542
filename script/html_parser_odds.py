# -*- coding: utf-8 -*-
# for 2.7

import html_parser_utility
import function_utility

#   @brief  1レース分のオッズデータを切り出すclass
class OddsParserAtRace(html_parser_utility.HTMLParser):
 
    def __init__(self):
        html_parser_utility.HTMLParser.__init__(self)
        #
        self.tag_now = ''       # 注目タグ
        self.b_result = False   # 処理成否
        #
        self.data_proc = function_utility.blankFunc()           # データ処理関数
        self.start_parser = self.start_parser_get_content_title # 開始タグparser
        self.end_parser = function_utility.blankFunc()          # 終了タグparser
        self.comment_proc = function_utility.blankFunc()        # コメント検出関数
        # 
        self.year = 0   # 西暦年
        self.month = 0  # 西暦月
        self.day = 0    # 西暦日
        # テンポラリ
        self.tbl_tr_cnt = 0 # trカウンタ
        self.tbl_td_cnt = 0 # tdカウンタ
        self.mtx = [[0, 0, 0, 0],
                    [0, 0, 0, 0],
                    [0, 0, 0, 0],
                    [0, 0, 0, 0],
                    [0, 0, 0, 0],
                    [0, 0, 0, 0],
                    [0, 0, 0, 0]]
        #
        self.odds_wins3 = []    # オッズデータ(3連単)
        self.odds_wins2 = []    # オッズデータ(2連単)
        self.odds_place3 = []   # オッズデータ(3連複)
        self.odds_place2 = []   # オッズデータ(2連複)
        
    def handle_starttag(self, tag, attrs):
        self.tag_now = tag
        if not function_utility.isBlankFunc(self.start_parser):
            self.start_parser(tag, attrs)

    def handle_endtag(self, tag):
        self.tag_now = tag
        if not function_utility.isBlankFunc(self.end_parser):
            self.end_parser(tag)

    def handle_data(self, data):
        if not function_utility.isBlankFunc(self.data_proc):
            self.data_proc(data)

    def handle_comment(self, data):
        if not function_utility.isBlankFunc(self.comment_proc):
            self.comment_proc(data)

    #   @brief  開始タグparser:<h2 class="content-title">をフック
    #   @note   解析開始位置検出
    def start_parser_get_content_title(self, tag, attrs):
        if tag == 'h2':
            attrs = dict(attrs)
            if 'class' in attrs and attrs['class'] == 'content-title':
                self.data_proc = self.data_proc_get_date
                self.start_parser = self.start_parser_find_footer

    #   @brief  コメント検出:<!-- cache kaisai -->をフック
    #   @note   解析開始位置検出
    #   @note   → タイミングによって欠落することがあるので廃止
    def comment_proc_kaisai(self, data):
        if data == ' cache kaisai ':
            self.data_proc = self.data_proc_get_date
            self.comment_proc = function_utility.blankFunc()

    #   @brief  データ処理関数:西暦年月日取得
    def data_proc_get_date(self, data):
        if self.tag_now == 'h3':
            data_u8 = data.decode('utf-8')
            inx_year = data_u8.find(u'年')
            inx_mon = data_u8.find(u'月')
            inx_day = data_u8.find(u'日')
            if inx_year > 0 and inx_mon > 0 and inx_day > 0:
                self.year = int(data_u8[0:inx_year])
                self.month = int(data_u8[inx_year+1:inx_mon])
                self.day = int(data_u8[inx_mon+1:inx_day])
                self.data_proc = self.data_proc_get_category
            else:
                # エラー
                self.data_proc = function_utility.blankFunc()

    #   @brief  開始タグparser:<footer class="post-footer">をフック
    #   @note   データが丸々存在しない場合（開催中止等）の対応
    def start_parser_find_footer(self, tag, attrs):
        if tag == 'footer':
            attrs = dict(attrs)
            if 'class' in attrs and attrs['class'] == 'post-footer':
                self.data_proc = function_utility.blankFunc()
                self.start_parser = function_utility.blankFunc()
                # 成功扱い
                self.b_result = True

    #   @brief  コメント検出:<!-- cache kekka -->をフック
    #   @note   オッズデータ開始位置検出
    #   @note   → タイミングによって欠落することがあるので廃止
    def comment_proc_kekka(self, data):
        if data == ' cache kekka ':
            self.data_proc = self.data_proc_get_category
            self.comment_proc = function_utility.blankFunc()

    #   @brief  データ処理関数:オッズデータカテゴリ取得
    def data_proc_get_category(self, data):
        if self.tag_now == 'b':
            data_u8 = data.decode('utf-8')
            # オッズデータ種別で分岐
            if data_u8 == u'3連単':
                self.data_proc = function_utility.blankFunc()
                self.start_parser =  self.start_parser_find_oddstable_wins3
            elif data_u8 == u'2連単':
                self.data_proc = function_utility.blankFunc()
                self.start_parser =  self.start_parser_find_oddstable_wins2
            elif data_u8 == u'3連複':
                self.data_proc = function_utility.blankFunc()
                self.start_parser =  self.start_parser_find_oddstable_place3
            elif data_u8 == u'2連複':
                self.data_proc = function_utility.blankFunc()
                self.start_parser =  self.start_parser_find_oddstable_place2
            elif data_u8 == u'結果':
                # 収集完了
                self.data_proc = function_utility.blankFunc()
                self.start_parser =  function_utility.blankFunc()
                self.end_parser =  function_utility.blankFunc()
                self.comment_proc =  function_utility.blankFunc()
                self.b_result = True
            else:
                # error
                self.data_proc = function_utility.blankFunc()
                self.start_parser =  function_utility.blankFunc()
                self.end_parser =  function_utility.blankFunc()
                self.comment_proc =  function_utility.blankFunc()
        elif self.tag_now == 'br':
            data_u8 = data.decode('utf-8')
            # オッズデータが欠落してることもある
            if u'オッズデータがありません' in data_u8:
                self.data_proc = function_utility.blankFunc()
                self.start_parser =  function_utility.blankFunc()
                self.end_parser =  function_utility.blankFunc()
                self.comment_proc =  function_utility.blankFunc()
                self.b_result = True

    #   @brief  開始タグparser:oddstable先頭検出(3連単用)
    def start_parser_find_oddstable_wins3(self, tag, attrs):
        if tag == 'div':
            attrs = dict(attrs)
            if 'class' in attrs and attrs['class'] == 'oddstable':
                self.data_proc =  self.data_proc_get_oddsdata_wins3
                self.end_parser = self.end_parser_get_oddsdata_wins3
                self.tbl_tr_cnt = 0
                self.tbl_td_cnt = 0
                
    #   @brief  データ処理関数:オッズデータ取得(3連単)
    def data_proc_get_oddsdata_wins3(self, data):
        if data and not data == '\n':
            if self.tbl_tr_cnt == 1:
                # 1連目は<td>カウンタから算出
                win1 = int(self.tbl_td_cnt / 3 + 1)
                # 2連目先頭<tr>
                tbl_td_inx = self.tbl_td_cnt % 3
                if tbl_td_inx == 0:
                    # 2連目取得
                    self.mtx[win1][2] = int(data)
                if tbl_td_inx == 1:
                    # 3連目取得
                    self.mtx[win1][3] = int(data)
                elif tbl_td_inx == 2:
                    # ((1,2,3連目), オッズ)で格納
                    self.odds_wins3.append([[win1, self.mtx[win1][2], self.mtx[win1][3]], float(data)])
            elif 2 <= self.tbl_tr_cnt and self.tbl_tr_cnt <= 4:
                # 1連目は<td>カウンタから算出
                win1 = int(self.tbl_td_cnt / 2 + 1)
                #
                tbl_td_inx = self.tbl_td_cnt % 2
                if tbl_td_inx == 0:
                    # 3連目取得
                    self.mtx[win1][3] = int(data)
                elif tbl_td_inx == 1:
                    # ((1,2,3連目), オッズ)で格納
                    self.odds_wins3.append([[win1, self.mtx[win1][2], self.mtx[win1][3]], float(data)])
        
    #   @brief  終了タグparser:オッズデータ取得(3連単)
    def end_parser_get_oddsdata_wins3(self, tag):
        if tag == 'tr':
            self.tbl_td_cnt = 0
            self.tbl_tr_cnt += 1
            if self.tbl_tr_cnt >= 5:
                self.tbl_tr_cnt -= 4 # 1～4でループ(3連目は最大4種類)
        elif tag == 'td':
            self.tbl_td_cnt += 1
        elif tag == 'table':
            # 3連単収集完了
            self.data_proc = self.data_proc_get_category
            self.start_parser =  function_utility.blankFunc()
            self.end_parser =  function_utility.blankFunc()


    #   @brief  開始タグparser:oddstable先頭検出(2連単用)
    def start_parser_find_oddstable_wins2(self, tag, attrs):
        if tag == 'div':
            attrs = dict(attrs)
            if 'class' in attrs and attrs['class'] == 'oddstable':
                self.data_proc =  self.data_proc_get_oddsdata_wins2
                self.end_parser = self.end_parser_get_oddsdata_wins2
                self.tbl_tr_cnt = 0
                self.tbl_td_cnt = 0
                
    #   @brief  データ処理関数:オッズデータ取得(2連単)
    def data_proc_get_oddsdata_wins2(self, data):
        if data and not data == '\n':
            if self.tbl_tr_cnt == 0:
                # 1連目は<td>カウンタから算出
                win1 = int(self.tbl_td_cnt / 3 + 1)
                # 1連目先頭<tr>
                tbl_td_inx = self.tbl_td_cnt % 3
                if tbl_td_inx == 1:
                    # 2連目取得
                    self.mtx[win1][2] = int(data)
                elif tbl_td_inx == 2:
                    # ((1,2連目), オッズ)で格納
                    self.odds_wins2.append([[win1, self.mtx[win1][2]], float(data)])
            elif 1 <= self.tbl_tr_cnt and self.tbl_tr_cnt <= 4:
                # 1連目は<td>カウンタから算出
                win1 = int(self.tbl_td_cnt / 2 + 1)
                #
                tbl_td_inx = self.tbl_td_cnt % 2
                if tbl_td_inx == 0:
                    # 2連目取得
                    self.mtx[win1][2] = int(data)
                elif tbl_td_inx == 1:
                    # ((1,2連目), オッズ)で格納
                    self.odds_wins2.append([[win1, self.mtx[win1][2]], float(data)])
        
    #   @brief  終了タグparser:オッズデータ取得(2連単)
    def end_parser_get_oddsdata_wins2(self, tag):
        if tag == 'tr':
            self.tbl_td_cnt = 0
            self.tbl_tr_cnt += 1
        elif tag == 'td':
            self.tbl_td_cnt += 1
        elif tag == 'table':
            # 2連単収集完了
            self.data_proc = self.data_proc_get_category
            self.start_parser =  function_utility.blankFunc()
            self.end_parser =  function_utility.blankFunc()


    #   @brief  開始タグparser:oddstable先頭検出(3連複用)
    def start_parser_find_oddstable_place3(self, tag, attrs):
        if tag == 'div':
            attrs = dict(attrs)
            if 'class' in attrs and attrs['class'] == 'oddstable':
                self.data_proc =  self.data_proc_get_oddsdata_place3
                self.end_parser = self.end_parser_get_oddsdata_place3
                self.tbl_td_cnt = 0
                
    #   @brief  データ処理関数:オッズデータ取得(3連複)
    def data_proc_get_oddsdata_place3(self, data):
        if data and not data == '\n':
            tbl_td_inx = self.tbl_td_cnt % 4
            if tbl_td_inx == 0:
                # 1連目取得
                self.mtx[0][1] = int(data)
            elif tbl_td_inx == 1:
                # 2連目取得
                self.mtx[0][2] = int(data)
            elif tbl_td_inx == 2:
                # 3連目取得
                self.mtx[0][3] = int(data)
            elif tbl_td_inx == 3:
                # ((1,2,3連目), オッズ)で格納
                self.odds_place3.append([[self.mtx[0][1], self.mtx[0][2], self.mtx[0][3]], float(data)])
        
    #   @brief  終了タグparser:オッズデータ取得(3連複)
    def end_parser_get_oddsdata_place3(self, tag):
        if tag == 'tr':
            self.tbl_td_cnt = 0
        elif tag == 'td':
            self.tbl_td_cnt += 1
        elif tag == 'table':
            # 3連複収集完了
            self.data_proc = self.data_proc_get_category
            self.start_parser =  function_utility.blankFunc()
            self.end_parser =  function_utility.blankFunc()


    #   @brief  開始タグparser:oddstable先頭検出(2連複用)
    def start_parser_find_oddstable_place2(self, tag, attrs):
        if tag == 'div':
            attrs = dict(attrs)
            if 'class' in attrs and attrs['class'] == 'oddstable':
                self.data_proc =  self.data_proc_get_oddsdata_place2
                self.end_parser = self.end_parser_get_oddsdata_place2
                self.tbl_td_cnt = 0
                
    #   @brief  データ処理関数:オッズデータ取得(2連複)
    def data_proc_get_oddsdata_place2(self, data):
        if data and not data == '\n':
            tbl_td_inx = self.tbl_td_cnt % 3
            if tbl_td_inx == 0:
                # 1連目取得
                self.mtx[0][1] = int(data)
            elif tbl_td_inx == 1:
                # 2連目取得
                self.mtx[0][2] = int(data)
            elif tbl_td_inx == 2:
                # ((1,2連目), オッズ)で格納
                self.odds_place2.append([[self.mtx[0][1], self.mtx[0][2]], float(data)])
        
    #   @brief  終了タグparser:オッズデータ取得(2連複)
    def end_parser_get_oddsdata_place2(self, tag):
        if tag == 'tr':
            self.tbl_td_cnt = 0
        elif tag == 'td':
            self.tbl_td_cnt += 1
        elif tag == 'table':
            # 2連複収集完了
            self.data_proc = self.data_proc_get_category
            self.start_parser =  function_utility.blankFunc()
            self.end_parser =  function_utility.blankFunc()


def parseOddstable(html_u8):

    parser = OddsParserAtRace()
    parser.feed(html_u8)
    parser.close()

    return (parser.b_result,
            parser.year,
            parser.month,
            parser.day,
            parser.odds_wins3,
            parser.odds_wins2,
            parser.odds_place3,
            parser.odds_place2)

def debugOutputHTMLToFile(html_u8, filename):

    html_parser_utility.debugOutputHTMLToFile(html_u8, filename)


'''
if __name__ == "__main__":
    
    #strbuff = open('../testdata/odds_2009_06_23_ST01_R11_MODE99.html').read()
    #strbuff = open('../testdata/odds_2009_06_23_ST14_R01_MODE99.html').read()
    #strbuff = open('../testdata/odds_2009_07_18_ST13_R01_MODE99.html').read()
    #strbuff = open('../testdata/odds_2009_10_08_ST13_R01_MODE99.html').read()
    #strbuff = open('../testdata/odds_2011_03_16_ST02_R01_MODE99.html').read()
    strbuff = open('../testdata/odds_2011_04_04_ST07_R01_MODE99.html').read()
    #strbuff = open('../testdata/odds_2018_01_01_ST06_R01_MODE99.html').read()
    parseOddstable(strbuff)
'''