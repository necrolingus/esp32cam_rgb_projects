from flask import Flask, request, jsonify
import numpy as np
from PIL import Image
import base64
import io
import json
import random
import string


app = Flask(__name__)


def generate_random_string(length):
	characters = string.ascii_lowercase + string.digits  # All lowercase letters and digits
	random_string = ''.join(random.choices(characters, k=length))
	return random_string
	


def get_rgb(image_data):
	# Create a BytesIO object from the bytes object
	image_stream = io.BytesIO(image_data)

	# Load the image from the BytesIO object
	img = Image.open(image_stream)
	
	# Convert image to RGB if it's not already
	img = img.convert('RGB')

	# Get dimensions
	width, height = img.size

	# Calculate the size of each block
	block_width = width // 3
	block_height = height // 3

	# Convert image to a numpy array
	img_array = np.array(img)

	# Initialize a list to hold the average RGB values of each block
	average_colors = []
	
	# Process each block
	for i in range(3):  # row index
		for j in range(3):  # column index
			# Calculate the block's x and y coordinates
			x_start = j * block_width
			y_start = i * block_height
			x_end = x_start + block_width
			y_end = y_start + block_height

			# Extract the block
			block = img_array[y_start:y_end, x_start:x_end]

			# Calculate the average color
			average_color = np.mean(block, axis=(0, 1)).astype(int)
			average_colors.append(average_color)

	# Put results into a JOSN variable
	json_rgb = {}
	for idx, color in enumerate(average_colors):
		r, g, b = color
		json_rgb.setdefault(idx, [str(r),str(g),str(b)])
		
	return json_rgb

@app.route('/processimage', methods=['POST'])
def process_image():
	if request.data:
		# Convert base64 byte string to a bytes object
		image_data = base64.b64decode(request.data)
		file_name = generate_random_string(10)
		json_rgb = get_rgb(image_data)
		
		with open(file_name + '.jpg', 'wb') as image_file:
			image_file.write(image_data)  # Write the binary image data to a file
			
		with open(file_name + '.json', 'w') as json_file:
			json_file.write(json.dumps(json_rgb, indent=4))  # Write the json data with the image
		
		return jsonify(json_rgb)
	else:
		return "No data received", 400

if __name__ == '__main__':
	app.run(host='0.0.0.0', port=35201, debug=True)
	