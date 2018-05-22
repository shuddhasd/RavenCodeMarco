import sip,sys,os,pty,time
import signal
from kivy.app import App
from kivy.uix.label import Label
from kivy.uix.widget import Widget
from kivy.uix.dropdown import DropDown
from kivy.uix.button import Button
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.checkbox import CheckBox
import subprocess
import threading
import time
from kivy.properties import StringProperty
from kivy.properties import ListProperty
from kivy.properties import NumericProperty
from kivy.properties import BooleanProperty
from kivy.config import Config

Config.set('graphics', 'width', '1280')
Config.set('graphics', 'height', '720')
Config.set('graphics', 'left', 0)
Config.set('graphics', 'top',  0)	

dropdown = DropDown();
btn_max = Button(text='max', size_hint_y=None, height=30)
btn_max.bind(on_release=lambda btn: dropdown.select(btn_max.text))
dropdown.add_widget(btn_max)
btn_max_notrigger = Button(text='max_notrigger', size_hint_y=None, height=30)
btn_max_notrigger.bind(on_release=lambda btn: dropdown.select(btn_max_notrigger.text))
dropdown.add_widget(btn_max_notrigger)
btn_max_notrigger_cluster = Button(text='max_notrigger_cluster', size_hint_y=None, height=30)
btn_max_notrigger_cluster.bind(on_release=lambda btn: dropdown.select(btn_max_notrigger_cluster.text))
dropdown.add_widget(btn_max_notrigger_cluster)
btn_max_notrigger_gainscan = Button(text='max_notrigger_gainscan', size_hint_y=None, height=30)
btn_max_notrigger_gainscan.bind(on_release=lambda btn: dropdown.select(btn_max_notrigger_gainscan.text))
dropdown.add_widget(btn_max_notrigger_gainscan)
btn_testmapping = Button(text='testmapping', size_hint_y=None, height=30)
btn_testmapping.bind(on_release=lambda btn: dropdown.select(btn_testmapping.text))
dropdown.add_widget(btn_testmapping)
btn_All = Button(text='All', size_hint_y=None, height=30)
btn_All.bind(on_release=lambda btn: dropdown.select(btn_All.text))
dropdown.add_widget(btn_All)


nlineF = 16
nsectionF = 3
line = 1.0/((nlineF*1.5)+(0.7*nsectionF)+0.5)
space = line*0.5
linespace = line+space
separator = line*1.2
vspace = 1.0/4
mainbutton = Button(text='Select Mode', size_hint=(2*vspace-0.01, line),pos_hint={'right': 4*vspace , 'top': 1-(11*linespace+2*separator-space)} )
mainbutton.bind(on_release=dropdown.open)
dropdown.bind(on_select=lambda instance, x: setattr(mainbutton, 'text', x))



class general_layout(FloatLayout):

	col1 = ListProperty([1., 0., 0.])
	col2 = ListProperty([1., 0., 0.])
	col3 = ListProperty([1., 0., 0.])

	def __init__(self, **kwargs):
		super(general_layout, self).__init__(**kwargs)
		self.add_widget(mainbutton)

		self.col1=[1,0,0]
		self.col2=[1,0,0]
		self.col3=[1,0,0]

	def update_pedestal_color_green(self):
		self.ids.pedestal_done.text = 'Complete'
		self.ids.pedestal_done.color = 0,1,0,1
		self.col1 = [0,1,0]

	def update_pedestal_color_red(self):
		self.ids.pedestal_done.text = 'To Do'
		self.ids.pedestal_done.color = 1,0,0,1
		self.col1 = [1,0,0]

	def update_decoder_color_green(self):
		self.ids.decoder_done.text = 'Complete'
		self.ids.decoder_done.color = 0,1,0,1
		self.col2 = [0,1,0]

	def update_decoder_color_red(self):
		self.ids.decoder_done.text = 'To Do'
		self.ids.decoder_done.color = 1,0,0,1
		self.col2 = [1,0,0]

	def update_analysis_color_green(self):
		self.ids.analysis_done.text = 'Complete'
		self.ids.analysis_done.color = 0,1,0,1
		self.col3 = [0,1,0]

	def update_analysis_color_red(self):
		self.ids.analysis_done.text = 'To Do'
		self.ids.analysis_done.color = 1,0,0,1
		self.col3 = [1,0,0]

	'''terminal_text = StringProperty()

	def update_terminal(self,text):
		terminal_label = self.ids['terminal_out']
		terminal_label.text = text

	def terminal_work(self,comm):
		p = subprocess.Popen([comm], stdout=subprocess.PIPE)
		while True:
			terminal_line = p.stdout.readline()
			print terminal_line
			threading.Thread(target=self.update_terminal(terminal_line)).start()
			sys.stdout.flush()
			if 'COMPLETE' in terminal_line:
				break
		p.kill()
	'''
	def pedestalcommand(self,in1,in2,in3,in4,in5):
		self.update_pedestal_color_red()
		pedestal_settings = open("pedestal_analysis_settings","w")
		pedestal_settings.write(in1+"\n"+in2+"\n"+in3)
		pedestal_settings.close()
		time.sleep(1)
		os.system("./ravenpedestal")
		self.update_pedestal_color_green()

	def decodingcommand(self,in1,in2,in3,in4,in5,in6,in7):
		self.update_decoder_color_red()
		decoder_settings = open("decoder_settings","w")
		decoder_settings.write(in1+"\n"+in2+"\n"+in3+"\n"+in4+"\n"+in5+"\n"+in6+"\n"+str(int(in7)))
		decoder_settings.close()
		time.sleep(1)
		os.system("./ravendecoder")
		self.update_decoder_color_green()

	def analysiscommand(self,in1,in2,in4):
		in3 = mainbutton.text
		self.update_analysis_color_red()
		analyzer_settings = open("analyzer_settings","w")
		analyzer_settings.write(in1+"\n"+in2+"\n"+in3+"\n"+in4)
		print in1+"\n"+in2+"\n"+in3+"\n"+in4
		analyzer_settings.close()
		time.sleep(1)
		os.system("./ravenanalyzer")
		self.update_analysis_color_green()

	def updatecommand(self, in1, in2, in3, in4, in5):
		updater_settings = open("detector_settings","w")
		updater_settings.write(in1+"\n"+in2+"\n128\n"+in3+"\n"+in4+"\n"+in5)
		print in1+"\n"+in2+"\n128\n"+in3+"\n"+in4+"\n"+in5
		updater_settings.close()
		time.sleep(1)

class ravengui(App):
	def build(self):
		self.load_kv("ravenlayout.kv")
		return general_layout()

if __name__ == "__main__":
    ravengui().run()


