extends Control

signal status_change(status)

var chess = preload("res://bin/chess.gdns").new()
var dragging = false
var dragged
var start_pos
var capturing := false

export (AudioStream) var checkmate_sound
export (AudioStream) var check_sound
export (AudioStream) var capture_sound
export (AudioStream) var white_move_sound
export (AudioStream) var black_move_sound

const frames = {
	"white king": 0,
	"white queen": 1,
	"white bishop": 2,
	"white knight": 3,
	"white rook": 4,
	"white pawn": 5,
	"black king": 6,
	"black queen": 7,
	"black bishop": 8,
	"black knight": 9,
	"black rook": 10,
	"black pawn": 11,
	"empty": -1
}


func _init():
	_setup_pieces()


func _setup_pieces():
	for i in get_children():
		if not i is AudioStreamPlayer:
			i.queue_free()
		
	var state = chess.board_state()
	for i in range (0,64):
		if frames[state[i]] == -1:
			continue
			
		var piece = preload("res://Button.tscn").instance()
		var sprite = piece.get_node("Sprite")
		sprite.frame = frames[state[i]]
		piece.grid_pos = Vector2(floor(i/8), i%8)
		piece.margin_left = piece.grid_pos.y * 75
		piece.margin_top = piece.grid_pos.x * 75
		piece.rect_size = Vector2(75, 75)
		piece.connect("grabbed", self, "_on_Button_grabbed")
		if state[i].begins_with("black"):
			if chess.is_white_turn():
				piece.disabled = true
		else:
			if not chess.is_white_turn():
				piece.disabled = true
		add_child(piece)


func _on_Button_grabbed(button):
	dragging = true
	dragged = button
	dragged.get_node("Sprite").z_index = 1
	start_pos = dragged.get_position()


func _input(event):
	if dragging and event is InputEventMouseButton and event.button_index == BUTTON_LEFT and !event.pressed:
		dragging = false
		dragged.get_node("Sprite").z_index = 0
		var new_grid_pos = _grid_pos(dragged.get_position())
		capturing = chess.board_state()[new_grid_pos.x*8+new_grid_pos.y] != "empty"
		if chess.move(dragged.grid_pos, new_grid_pos):
			_on_move()
		else:
			dragged.set_position(start_pos)


func _on_move():
	_setup_pieces()
	if chess.is_stalemated():
		emit_signal("status_change", "Stalemate")
	elif chess.is_checkmated():
		var new_status = "Black wins!" if chess.is_white_turn() else "White wins!"
		emit_signal("status_change", new_status)
	else:
		var new_status = "White" if chess.is_white_turn() else "Black"
		new_status += " in Check!" if chess.is_in_check() else " Move"
		emit_signal("status_change", new_status)
	_play_sound_effect()


func _play_sound_effect():
	var sound: AudioStream
	if chess.is_checkmated():
		sound = checkmate_sound
	elif chess.is_in_check():
		sound = check_sound
	elif capturing:
		sound = capture_sound
	elif chess.is_white_turn(): # black just went
		sound = black_move_sound
	else:
		sound = white_move_sound
	
	var player = $AudioStreamPlayer
	player.stop()
	player.stream = sound
	player.play()


func _process(delta):
	if dragging:
		var new_position = get_viewport().get_mouse_position()
		new_position.x -= margin_right
		new_position.x += dragged.rect_size.x / 2
		new_position.y -= dragged.rect_size.y / 2
		dragged.set_position(new_position)


func _grid_pos(pos: Vector2):
	var x = floor((pos.y + 32) / 75)
	var y = floor((pos.x + 32) / 75)
	return Vector2(x, y)
