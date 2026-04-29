- ADX Core V0

## DFM Check
- PCB DFM
	- フットプリント以外でGood以下のもの無し
- SMT DFM
	- ATtinyのチップ裏のviaが大丈夫なのか不明
		- とりあえずデータシートの推奨値ではあるものの製造できるのか不明
		- また、EasyEDAでtentedを明示的に指定できないため、Solder Maskの-1000milなどで指定しているがこれもどうなるかはっきりしない。3Dビューでもいまいちはっきりしたことは分からない。
		- その他、ATtinyのピン部分やチップ部分はSMT DFMの推奨外が多い。
		- 