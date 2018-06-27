#! /bin/bash
reldir=`dirname $0`
echo $reldir
cd $reldir
echo 'This Program needs ROOT 6.10, GCC 5.4.0, python 2.7 and kivy 1.9.'
echo 'Compiling Raven'
g++ -o ravenpedestal ravenpedestal.C `root-config --cflags --glibs` -l TreePlayer
echo 'Raven Pedestal compiled'
g++ -o ravendecoder ravendecoder.C `root-config --cflags --glibs` -l TreePlayer
echo 'Raven Decoder compiled'
g++ -o ravenanalyzer ravenanalyzer.C `root-config --cflags --glibs` -l TreePlayer
echo 'Raven Analyzer compiled'
echo 'Compilation Complete'
echo 'Open gui'
python ravengui.py