extends Button


var grid_pos = Vector2()
signal grabbed(button)


func _on_Button_button_down():
	emit_signal("grabbed", self)
