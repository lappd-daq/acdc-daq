#!/usr/bin/env python
import os
import sys

import numpy as np
from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QFileDialog

from gui import Ui_MainWindow

acdc_settings = {
    'b0': {
        'ped': [0x800, 0x800, 0x800, 0x800, 0x800],
        'thresh': [0x000, 0x000, 0x000, 0x000, 0x000]
    },
    'b1': {
        'ped': [0x800, 0x800, 0x800, 0x800, 0x800],
        'thresh': [0x000, 0x000, 0x000, 0x000, 0x000]
    },
    'b2': {
        'ped': [0x800, 0x800, 0x800, 0x800, 0x800],
        'thresh': [0x000, 0x000, 0x000, 0x000, 0x000]
    },
    'b3': {
        'ped': [0x800, 0x800, 0x800, 0x800, 0x800],
        'thresh': [0x000, 0x000, 0x000, 0x000, 0x000]
    },
    'b4': {
        'ped': [0x800, 0x800, 0x800, 0x800, 0x800],
        'thresh': [0x000, 0x000, 0x000, 0x000, 0x000]
    },
    'b5': {
        'ped': [0x800, 0x800, 0x800, 0x800, 0x800],
        'thresh': [0x000, 0x000, 0x000, 0x000, 0x000]
    },
    'b6': {
        'ped': [0x800, 0x800, 0x800, 0x800, 0x800],
        'thresh': [0x000, 0x000, 0x000, 0x000, 0x000]
    },
    'b7': {
        'ped': [0x800, 0x800, 0x800, 0x800, 0x800],
        'thresh': [0x000, 0x000, 0x000, 0x000, 0x000]
    }
}

trig_settings = {
    'b0': {
        'mask': 0x0,
        'en': 0,
        'sign': 0,
        'sma_trig': 0
    },
    'b1': {
        'mask': 0x0,
        'en': 0,
        'sign': 0,
        'sma_trig': 0
    },
    'b2': {
        'mask': 0x0,
        'en': 0,
        'sign': 0,
        'sma_trig': 0
    },
    'b3': {
        'mask': 0x0,
        'en': 0,
        'sign': 0,
        'sma_trig': 0
    },
    'b4': {
        'mask': 0x0,
        'en': 0,
        'sign': 0,
        'sma_trig': 0
    },
    'b5': {
        'mask': 0x0,
        'en': 0,
        'sign': 0,
        'sma_trig': 0
    },
    'b6': {
        'mask': 0x0,
        'en': 0,
        'sign': 0,
        'sma_trig': 0
    },
    'b7': {
        'mask': 0x0,
        'en': 0,
        'sign': 0,
        'sma_trig': 0
    },
    'wait_for_sys': 0,
    'rate_only': 0,
    'use_coinc': 0,
    'use_trig_valid': 0,
    'coinc_window': 6,  # < 15
    'coinc_pulsew': 2,
    'coinc_num_asic': 1,
    'coinc_num_ch': 1,
}


def get_acdc_settings():
    for b, board in enumerate(list(acdc_settings)):
        acdc_settings[board]['ped'] = [eval('ui.Ped_{0}_{1}.value()'.format(b, chip))
                                       for chip in range(1, 6)]
        acdc_settings[board]['thresh'] = [eval('ui.thresh_{0}_{1}.value()'.format(b, chip))
                                          for chip in range(1, 6)]


def write_acdc_settings():
    get_acdc_settings()
    filename, _ = QFileDialog.getSaveFileName(
        ui.export_acdc, 'Save File', os.getcwd())
    f = open(filename, 'w')

    for b, board in enumerate(list(acdc_settings)):
        f.write('# ACDC Settings for {}\n'.format(board))
        for ch, ped in enumerate(acdc_settings[board]['ped']):
            f.write('pedestal\t{0}\t{1}\t{2}\n'.format(b, ch, ped))
        f.write('\n')
        for ch, thresh in enumerate(acdc_settings[board]['thresh']):
            f.write('threshold\t{0}\t{1}\t{2}\n'.format(b, ch, thresh))
        f.write('\n')
    f.close()


###############################################################################
# trig Parsing Functions
#
#
#
"""
Will return a hex mask given an array of booleans for on/off masking of channels

Parameters:
    buttons- a boolean array ordered from channel 1 to channel 30

Note that the binary string represantation of the input boolean array is actually reversed
"""


def get_mask():
    boards = [[eval('ui.ch{}button_{}.isChecked()'.format(ch, b)) for ch in range(1, 31)] for b in range(8)]

    out = ''
    for board, channels in enumerate(boards):
        bin_string = ''
        for on in channels:
            bin_string = ('1' if on else '0') + bin_string
        out += 'trig_mask\t{}\t{}\n'.format(board, hex(int(bin_string, 2)))

    return out


def get_trig_en():
    trigs = [eval('ui.trig_en_board_{}.isChecked()'.format(b)) for b in range(8)]

    out = ''
    for board, value in enumerate(trigs):
        on = '1' if value else '0'
        out += 'trig_enable\t{}\t{}\n'.format(board, on)
    return out


def get_trig_sign():
    signs = [eval('ui.trig_sign_{}.currentText()'.format(b)) for b in range(8)]
    out = ''
    for board, value in enumerate(signs):
        sign = '1' if value == '+' else '0'
        out += 'trig_sign\t{}\t{}\n'.format(board, sign)
    return out


def get_hwtrig_settings():
    hwtrig = ui.hardware_trig.isChecked()
    sources = [ui.hardware_src_ext.isChecked()]
    sources += [eval('ui.hardware_src_{}.isChecked()'.format(b)) for b in range(8)]
    on = '1' if hwtrig else '0'
    src = np.argmax(sources)
    if src > 0:
        src = src + 2
    return 'hrdw_trig\t{}\nhrdw_src_trig\t{}\n'.format(on, src)


def parse_trig():
    filename, _ = QFileDialog.getSaveFileName(
        ui.export_trig, 'Save File', os.getcwd())
    f = open(filename, 'w')
    # Masks
    f.write(get_mask() + '\n')
    f.write(get_trig_en() + '\n')
    f.write(get_trig_sign() + '\n')
    f.write(get_hwtrig_settings() + '\n')

    f.write(ui.extras_text.toPlainText())
    f.close()

###############################################################################
# clicked
#
#
#


def maskall(board):
    group = eval('ui.b{}_mask'.format(board))
    for button in group.buttons():
        button.setChecked(True)


def masknone(board):
    group = eval('ui.b{}_mask'.format(board))
    for button in group.buttons():
        button.setChecked(False)


def sync_ped(board):
    group = [eval('ui.Ped_{}_{}'.format(board, chip)) for chip in range(1, 6)]
    const = group[0].value()
    for text in group:
        text.setValue(const)


def sync_thresh(board):
    group = [eval('ui.thresh_{}_{}'.format(board, chip))
             for chip in range(1, 6)]
    const = group[0].value()
    for text in group:
        text.setValue(const)


def buttons():
    ui.export_acdc.clicked.connect(write_acdc_settings)
    ui.export_trig.clicked.connect(parse_trig)
    ui.mask_all_0.clicked.connect(lambda: maskall(0))
    ui.mask_all_1.clicked.connect(lambda: maskall(1))
    ui.mask_all_2.clicked.connect(lambda: maskall(2))
    ui.mask_all_3.clicked.connect(lambda: maskall(3))
    ui.mask_all_4.clicked.connect(lambda: maskall(4))
    ui.mask_all_5.clicked.connect(lambda: maskall(5))
    ui.mask_all_6.clicked.connect(lambda: maskall(6))
    ui.mask_all_7.clicked.connect(lambda: maskall(7))
    ui.mask_none_0.clicked.connect(lambda: masknone(0))
    ui.mask_none_1.clicked.connect(lambda: masknone(1))
    ui.mask_none_2.clicked.connect(lambda: masknone(2))
    ui.mask_none_3.clicked.connect(lambda: masknone(3))
    ui.mask_none_4.clicked.connect(lambda: masknone(4))
    ui.mask_none_5.clicked.connect(lambda: masknone(5))
    ui.mask_none_6.clicked.connect(lambda: masknone(6))
    ui.mask_none_7.clicked.connect(lambda: masknone(7))
    ui.sync_ped_0.clicked.connect(lambda: sync_ped(0))
    ui.sync_ped_1.clicked.connect(lambda: sync_ped(1))
    ui.sync_ped_2.clicked.connect(lambda: sync_ped(2))
    ui.sync_ped_3.clicked.connect(lambda: sync_ped(3))
    ui.sync_ped_4.clicked.connect(lambda: sync_ped(4))
    ui.sync_ped_5.clicked.connect(lambda: sync_ped(5))
    ui.sync_ped_6.clicked.connect(lambda: sync_ped(6))
    ui.sync_ped_7.clicked.connect(lambda: sync_ped(7))
    ui.sync_thresh_0.clicked.connect(lambda: sync_thresh(0))
    ui.sync_thresh_1.clicked.connect(lambda: sync_thresh(1))
    ui.sync_thresh_2.clicked.connect(lambda: sync_thresh(2))
    ui.sync_thresh_3.clicked.connect(lambda: sync_thresh(3))
    ui.sync_thresh_4.clicked.connect(lambda: sync_thresh(4))
    ui.sync_thresh_5.clicked.connect(lambda: sync_thresh(5))
    ui.sync_thresh_6.clicked.connect(lambda: sync_thresh(6))
    ui.sync_thresh_7.clicked.connect(lambda: sync_thresh(7))


###############################################################################
# Main Methods
#
#
#
if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    MainWindow = QtWidgets.QMainWindow()
    ui = Ui_MainWindow()
    ui.setupUi(MainWindow)
    buttons()

    MainWindow.show()
    sys.exit(app.exec_())
