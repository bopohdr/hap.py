#!/illumina/development/haplocompare/hc-virtualenv/bin/python
# coding=utf-8
#
# Copyright (c) 2010-2015 Illumina, Inc.
# All rights reserved.
#
# This file is distributed under the simplified BSD license.
# The full text can be found here (and in LICENSE.txt in the root folder of
# this distribution):
#
# https://github.com/sequencing/licenses/blob/master/Simplified-BSD-License.txt

import os
import abc
import pandas
import logging
import subprocess
import tempfile


def tableROC(tbl, label_column, feature_column, filter_column=None,
             filter_name=None):
    """Compute ROC table from TP/FP/FN classification table.

    :param tbl: table with label and feature
    :type tbl: pandas.DataFrame
    :param label_column: column name which gives the label (TP/FP/FN)
    :param feature_column: column name which gives the feature
    :param filter_column: column that contains the filter fields
    :param filter_name: column that contains the filter name
    :returns: a pandas.DataFrame with TP/FP/FN/precision/recall columns.
    """

    tf1 = tempfile.NamedTemporaryFile(delete=False)
    tf1.close()
    tf2 = tempfile.NamedTemporaryFile(delete=False)
    tf2.close()
    try:
        fields = [feature_column, label_column]
        if filter_column:
            fields.append(filter_column)

        tbl[fields].to_csv(tf2.name, sep="\t", index=False)

        cmdline = "roc -t %s -v %s --verbose " % (label_column, feature_column)
        if filter_column:
            cmdline += " -f %s" % filter_column
        if filter_name:
            cmdline += " -n %s" % filter_name
        cmdline += " -o %s %s" % (tf1.name, tf2.name)

        logging.info("Running %s" % cmdline)

        subprocess.check_call(cmdline, shell=True)
        try:
            result = pandas.read_table(tf1.name)
        except:
            raise Exception("Cannot parse ROC output.")
        return result
    finally:
        try:
            os.unlink(tf1.name)
        except:
            pass
        try:
            os.unlink(tf2.name)
        except:
            pass


class ROC(object):
    """ROC calculator base class"""

    __metaclass__ = abc.ABCMeta

    classes = {}

    features = {}

    def __init__(self):
        self.ftable = ""

    @abc.abstractmethod
    def from_table(self, tbl):
        """ Create ROC from feature table
        :param tbl: the table
        :type tbl: pandas.DataFrame
        :rtype: pandas.DataFrame
        """
        pass

    @classmethod
    def make(cls, cname):
        # noinspection PyCallingNonCallable
        c = cls.classes[cname]()
        c.ftname = cls.features[cname]
        return c

    @classmethod
    def register(cls, name, ftname, cons):
        """ Register a ROC calculator
        :param name: the name of the calculator
        :param ftname: the features / feature table name
                      (will be accessible in the ftname attribute)
        :param cons: class constructor
        """
        cls.classes[name] = cons
        cls.features[name] = ftname

    @classmethod
    def list(cls):
        return cls.classes.keys()


class StrelkaSNVRoc(ROC):
    """ROC calculator for Strelka SNVs"""

    def from_table(self, tbl):
        tbl.loc[tbl["NT"] != "ref", "QSS_NT"] = 0
        return tableROC(tbl, "tag",
                        "QSS_NT", "FILTER", "QSS_ref")

ROC.register("strelka.snv.qss", "hcc.strelka.snv", StrelkaSNVRoc)


class StrelkaSNVVQSRRoc(ROC):
    """ROC calculator for Strelka SNVs (newer versions which use VQSR)"""

    def from_table(self, tbl):
        tbl.loc[tbl["NT"] != "ref", "VQSR"] = 0
        return tableROC(tbl, "tag",
                        "VQSR", "FILTER", "LowQscore")

ROC.register("strelka.snv", "hcc.strelka.snv", StrelkaSNVVQSRRoc)


class StrelkaIndelRoc(ROC):
    """ROC calculator for Strelka Indels"""

    def from_table(self, tbl):
        # fix QSI for NT != ref
        tbl.loc[tbl["NT"] != "ref", "QSI_NT"] = 0
        return tableROC(tbl, "tag",
                        "QSI_NT", "FILTER", "QSI_ref")

ROC.register("strelka.indel", "hcc.strelka.indel", StrelkaIndelRoc)
