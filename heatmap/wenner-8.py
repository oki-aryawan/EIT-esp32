import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
from scipy import interpolate

# Data input for 8 electrodes (4 layers)
all_data = [265.20, 400.49, 451.77, 535.29, 622.71, 346.28, 545.54, 606.10, 687.67, 426.86, 631.01, 739.44, 578.75]


layer1_data = all_data[:5]   # 5 measurements
layer2_data = all_data[5:9]  # 4 measurements
layer3_data = all_data[9:12] # 3 measurements
layer4_data = all_data[12:]  # 1 measurement

# Combine all layers
wenner_data = [layer1_data, layer2_data, layer3_data, layer4_data]

# Create a 2D array to hold the data
max_width = len(wenner_data[0])
height = len(wenner_data)
plot_data = np.full((height, max_width), np.nan)

# Fill in the data
for i, row in enumerate(wenner_data):
    start = (max_width - len(row)) // 2
    plot_data[i, start:start+len(row)] = row

# Create a mask for valid data points
mask = ~np.isnan(plot_data)

# Create coordinates for the original data points
y, x = np.mgrid[0:height, 0:max_width]

# Create a fine mesh for interpolation (increase the resolution)
x_upsampled = np.linspace(0, max_width - 1, max_width * 10)
y_upsampled = np.linspace(0, height - 1, height * 10)
xx, yy = np.meshgrid(x_upsampled, y_upsampled)

# Perform the interpolation
points = np.column_stack((y[mask].ravel(), x[mask].ravel()))
values = plot_data[mask].ravel()
grid_z = interpolate.griddata(points, values, (yy, xx), method='cubic', fill_value=np.nan)

# Create the plot
fig, ax = plt.subplots(figsize=(12, 8))

# Plot the interpolated data
im = ax.imshow(grid_z, cmap='jet', norm=LogNorm(vmin=min(all_data), vmax=max(all_data)),
               extent=[0, max_width, height, 0], aspect='auto')

# Customize the plot
ax.set_title('8-Electrode Wenner Array Resistivity Profile', fontsize=16)
ax.set_xlabel('Distance', fontsize=12)
ax.set_ylabel('Depth', fontsize=12)

# Remove x-axis ticks
ax.set_xticks([])

# Set y-axis ticks and labels
ax.set_yticks(np.arange(len(wenner_data)))
ax.set_yticklabels([f'a={i+1}' for i in range(len(wenner_data))])

# Add colorbar
cbar = plt.colorbar(im)
cbar.set_label('Resistivity in ohm·m', rotation=270, labelpad=15)

# Add grid lines to show original measurement points
for i in range(len(wenner_data)):
    ax.axhline(y=i, color='w', linestyle=':', linewidth=0.5, alpha=0.5)
    for j in range(max_width):
        if not np.isnan(plot_data[i, j]):
            ax.axvline(x=j, color='w', linestyle=':', linewidth=0.5, alpha=0.5)

plt.tight_layout()
plt.show()
