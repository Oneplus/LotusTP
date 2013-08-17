# -*- encoding: utf8 -*-

from LTP import Segmentor
from LTP import Postagger
from LTP import NER
from LTP import Parser

engine1 = Segmentor("../../cws.model")
engine2 = Postagger("../../pos.model")
engine3 = NER("../../ner.model")
engine4 = Parser("../../parser.model")

segment_result = engine1.segment("我爱北京天安门")
print "|".join( segment_result )

postag_result = engine2.postag( segment_result )
print "|".join( postag_result )

ner_result = engine3.ner( segment_result, postag_result )
print "|".join( ner_result )

parser_result = engine4.parse( segment_result, postag_result )
print "|".join( [str(h) for h in parser_result.heads] )
print "|".join( parser_result.deprels )
