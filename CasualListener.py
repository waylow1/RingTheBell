import socketio

sio = socketio.Client()


@sio.on('horairesModifies')
def on_horaires_modifies(data):
    print('Horaires modifiés :', data)


sio.connect('http://192.168.0.27:8080')  
sio.wait()
