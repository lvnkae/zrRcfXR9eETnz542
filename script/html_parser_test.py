# -*- coding: utf-8 -*-
# for 2.7

def debugOutputHTML(html_u8, filename):

    f = open(filename, 'w')
    f.write(html_u8)
    f.close()
