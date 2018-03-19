# -*- coding: utf-8 -*-
# for 2.7

import html_parser_utility
import function_utility

#   @brief  1ヶ月分のスケジュールを切り出すclass
#   @note   SBI-mobile[バックアップ]サイト<汎用>
class ScheduleParserAtMonth(html_parser_utility.HTMLParser):
 
    def __init__(self):
        html_parser_utility.HTMLParser.__init__(self)
        #
        self.tag_now = ''       # 注目タグ
        self.b_result = False   # 処理成否
        #
        self.data_proc = function_utility.blankFunc()   # データ処理関数
        self.start_parser = self.start_parser_check_html# 開始タグparser
        self.end_parser = function_utility.blankFunc()  # 終了タグparser
        # 
        self.year = 0                   # 西暦年
        self.month = 0                  # 西暦月
        self.calendar_table = []        # カレンダーtable(html)ワーク(1ヶ月分)
        self.calendar_row = []          # カレンダーtable(html)ワーク(1行分)
        self.calendar_e_day = ''        # カレンダーtable(html)ワーク(1日分)
        self.calendar_stadium_id = 0    # カレンダーtable(html)ワーク(レース場ID)
        self.calendar_race_days = 0     # カレンダーtable(html)ワーク(レース開催日数)
        self.calendar_race_grade = ''   # カレンダーtable(html)ワーク(レースグレード)
        self.calendar_race_name = ''    # カレンダーtable(html)ワーク(レース名)
        #
        self.schedule = []              # スケジュールデータ
        self.race_name = []             # レース名データ
        
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

    #   @brief  開始タグparser：htmlチェック
    #   @note   意図したhtmlであるか調べる
    def start_parser_check_html(self, tag, attrs):
        if tag == 'span':
            attrs = dict(attrs)
            if 'class' in attrs and attrs['class'] == 'heading1_mainLabel':
                self.start_parser = self.start_parser_find_year
    
    #   @brief  開始タグparser：西暦年検出
    def start_parser_find_year(self, tag, attrs):
        if tag == 'select':
            attrs = dict(attrs)
            if 'name' in attrs and attrs['name'] == 'listSelectYear.selected':
                self.start_parser = self.start_parser_get_year

    #   @brief  開始タグparser：西暦年取得
    def start_parser_get_year(self, tag, attrs):
        if tag == 'option':
            attrs = dict(attrs)
            if 'selected' in attrs and 'value' in attrs:
                self.year = int(attrs['value'])
                self.start_parser = self.start_parser_find_month

    #   @brief  開始タグparser：西暦月検出
    def start_parser_find_month(self, tag, attrs):
        if tag == 'select':
            attrs = dict(attrs)
            if 'name' in attrs and attrs['name'] == 'listSelectMonth.selected':
                self.start_parser = self.start_parser_get_month

    #   @brief  開始タグparser：西暦月取得
    def start_parser_get_month(self, tag, attrs):
        if tag == 'option':
            attrs = dict(attrs)
            if 'selected' in attrs and 'value' in attrs:
                self.month = int(attrs['value'])
                self.start_parser = self.start_parser_find_region

    #   @brief  開始タグparser：地区単位のtable検出
    def start_parser_find_region(self, tag, attrs):
        if tag == 'span':
            attrs = dict(attrs)
            if 'class' in attrs and attrs['class'] == 'title6_mainLabel':
                # 初回はカレンダー要素を得る、2地区目からはレース開催データを直接得る
                if not self.calendar_table:
                    self.start_parser = self.start_parser_find_calendar
                else:
                    self.start_parser = self.start_parser_find_stadium
                    self.data_proc = function_utility.blankFunc()
                    self.end_parser = self.end_parser_find_stadium
        elif tag == 'div':
            attrs = dict(attrs)
            # title2(○月のスケジュール）が見つかったら全地区終了
            if 'class' in attrs and attrs['class'] == 'title2':
                self.start_parser = function_utility.blankFunc()
                self.data_proc = function_utility.blankFunc()
                self.end_parser = function_utility.blankFunc()
                # 集計処理
                self.b_result = self.create_schedule_data()

    #   @brief  開始タグparser：カレンダーtable検出
    def start_parser_find_calendar(self, tag, attrs):
        if tag == 'tr':
            attrs = dict(attrs)
            if 'class' in attrs and attrs['class'] == 'is-fs11 is-thColor9':
                self.start_parser = function_utility.blankFunc()
                self.data_proc = self.data_proc_get_calendar
                self.end_parser = self.end_parser_get_calendar

    #   @brief  データ処理関数:カレンダー要素取得
    def data_proc_get_calendar(self, data):
        if self.tag_now == 'th' or self.tag_now == 'br':
            self.calendar_e_day += data
    #   @brief  終了タグparser:カレンダー要素取得
    def end_parser_get_calendar(self, tag):
        if tag == 'th':    
            self.calendar_row.append(self.calendar_e_day)
            self.calendar_e_day = ''
        elif tag == 'tr':
            self.calendar_table.append(self.calendar_row[:])
            del self.calendar_row[:]
            self.start_parser = self.start_parser_find_stadium
            self.data_proc = function_utility.blankFunc()
            self.end_parser = self.end_parser_find_stadium

    #   @brief  開始タグparser：レース場table検出
    def start_parser_find_stadium(self, tag, attrs):
        if tag == 'tr':
            attrs = dict(attrs)
            if 'class' in attrs and attrs['class'] == 'is-fs12':
                self.start_parser = self.start_parser_find_stadium_id
                self.data_proc = function_utility.blankFunc()
                self.end_parser = function_utility.blankFunc()
                #
                self.calendar_stadium_id = 0
                self.calendar_race_days = 0
                self.calendar_race_grade = ''
                self.calendar_race_name = ''
    #   @brief  終了タグparser:レース場table検出
    def end_parser_find_stadium(self, tag):
        if tag == 'table':    
            self.start_parser = self.start_parser_find_region
            self.data_proc = function_utility.blankFunc()
            self.end_parser = function_utility.blankFunc()

    #   @brief  開始タグparser：レース場ID検出
    def start_parser_find_stadium_id(self, tag, attrs):
        if tag == 'th':
            attrs = dict(attrs)
            if 'class' in attrs and attrs['class'] == 'is-thColor10 is-arrow1':
                self.start_parser = self.start_parser_get_stadium_id
                self.data_proc = function_utility.blankFunc()
                self.end_parser = self.end_parser_get_stadium_id

    #   @brief  開始タグparser：レース場ID取得
    def start_parser_get_stadium_id(self, tag, attrs):
        if tag == 'a':
            attrs = dict(attrs)
            if 'href' in attrs:
                self.calendar_stadium_id = int(attrs['href'][-2:]) #末尾2文字がID
    #   @brief  終了タグparser:レース場ID取得
    def end_parser_get_stadium_id(self, tag):
        if tag == 'th':    
            self.calendar_row.append(self.calendar_stadium_id)
            self.start_parser = self.start_parser_get_racedata
            self.data_proc = self.data_proc_get_racedata
            self.end_parser = self.end_parser_get_racedata

    #   @brief  開始タグparser：レース開催データ取得
    def start_parser_get_racedata(self, tag, attrs):
        if tag == 'td':
            attrs = dict(attrs)
            if 'colspan' in attrs:
                self.calendar_race_days = int(attrs['colspan'])
            if 'class' in attrs:
                grade_tag = attrs['class']
                if grade_tag == 'is-gradeColorSG':
                    self.calendar_race_grade = 'SG'
                elif grade_tag == 'is-gradeColorG1':
                    self.calendar_race_grade = 'G1'
                elif grade_tag == 'is-gradeColorG2':
                    self.calendar_race_grade = 'G2'
                elif grade_tag == 'is-gradeColorG3':
                    self.calendar_race_grade = 'G3'
                elif grade_tag == 'is-gradeColorIppan':
                    self.calendar_race_grade = 'NORMAL'
                elif grade_tag == 'is-gradeColorLady':
                    self.calendar_race_grade = 'LADYS'
                elif grade_tag == 'is-gradeColorRookie':
                    self.calendar_race_grade = 'ROOKIE'
                elif grade_tag == 'is-gradeColorVenus':
                    self.calendar_race_grade = 'VENUS'
                elif grade_tag == 'is-gradeColorTakumi':
                    self.calendar_race_grade = 'MASTERS'
                else:
                    self.calendar_race_grade = 'ERROR'
    #   @brief  データ処理関数:レース開催データ取得
    def data_proc_get_racedata(self, data):
        if self.tag_now == 'a':
            self.calendar_race_name = data
    #   @brief  終了タグparser:レース開催データ取得
    def end_parser_get_racedata(self, tag):
        if tag == 'td':
            if self.calendar_race_days > 0:
                if self.calendar_race_name:
                    self.race_name.append(self.calendar_race_name)
                    race_name_id = len(self.race_name)
                else:
                    race_name_id = -1
                for days in range(0, self.calendar_race_days):
                    
                    self.calendar_row.append([self.calendar_race_grade,
                                              race_name_id,
                                              days+1])
                self.calendar_race_days = 0
                self.calendar_race_grade = ''
                self.calendar_race_name = ''
            else:
                self.calendar_row.append([])
        elif tag == 'tr':
            self.calendar_table.append(self.calendar_row[:])
            del self.calendar_row[:]
            self.start_parser = self.start_parser_find_stadium
            self.data_proc = function_utility.blankFunc()
            self.end_parser = self.end_parser_find_stadium

    #   @brief  1日(ついたち)か？
    def is_1st_day(self, day_u8):
        if day_u8 == u'1月':
            return True
        elif day_u8 == u'1火':
            return True
        elif day_u8 == u'1水':
            return True
        elif day_u8 == u'1木':
            return True
        elif day_u8 == u'1金':
            return True
        elif day_u8 == u'1土':
            return True
        elif day_u8 == u'1日':
            return True
        else:
            return False

    #   @brief  スケジュールデータ生成
    def create_schedule_data(self):
        #レース場は24箇所(2018/03/13時点)
        stadium_num = 24
        # カレンダーデータ -> 日付データ＋レース場ごとの開催データ
        if not len(self.calendar_table) == stadium_num + 1:
            return False
        calendar = self.calendar_table[0]
        calendar_len = len(calendar)
        #
        calendar_top = -1
        calendar_end = -1
        for inx in range(1, calendar_len):
            if self.is_1st_day(calendar[inx].decode('utf-8')):
                calendar_top = inx
                break
        if calendar_top < 0:
            return False
        for inx in range(calendar_top+1, calendar_len):
            if self.is_1st_day(calendar[inx].decode('utf-8')):
                calendar_end = inx
                break
        if calendar_end < 0:
            return False
        #
        for inx in range(calendar_top, calendar_end):
            schedule_at_day = []
            for std_inx in range(0, stadium_num):
                std_id = std_inx + 1
                std_race = self.calendar_table[std_id][inx]
                if len(std_race) > 0:
                    schedule_at_day.append([std_id, std_race[0], std_race[1], std_race[2]])
            self.schedule.append(schedule_at_day[:])
            del schedule_at_day[:]
        return True


def parseSchedule(html_u8):

    parser = ScheduleParserAtMonth()
    parser.feed(html_u8)
    parser.close()

    return (parser.b_result,
            parser.year,
            parser.month,
            parser.race_name,
            parser.schedule)

def debugOutputHTMLToFile(html_u8, filename):

    html_parser_utility.debugOutputHTMLToFile(html_u8, filename)


'''
if __name__ == "__main__":

    url = "https://www.boatrace.jp/owpc/pc/race/monthlyschedule?ym=201801"
    request = html_parser_utility.urllib2.Request(url)
    request.add_header('Accept', 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8');
    request.add_header('Content-Type', 'application/x-www-form-urlencoded; charset=UTF-8');
    request.add_header('User-Agent', 'Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/57.0.2987.133 Safari/537.36');

    response = html_parser_utility.urllib2.urlopen(request)
    
    parseSchedule(response.read())
    
    response.close()
'''