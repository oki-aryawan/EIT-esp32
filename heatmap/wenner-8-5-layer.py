import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
from scipy import interpolate

# Actual data input for 5 layers
all_data = [329.18, 504.03, 232.48, 375.09, 624.18,
            602.69, 526.01, 399.02, 769.23, 503.05,
            599.27, 696.46, 582.17, 828.82, 974.36]
all_data = [341.88, 505.49, 217.83, 359.46, 606.11,
            620.76, 511.36, 385.35, 755.56, 511.36,
            596.34, 691.58, 583.64, 822.95, 963.13]
all_data = [361.90, 494.75, 228.57, 356.04, 568.50,
            612.94, 493.28, 400.98, 697.92, 539.19,
            601.22, 645.67, 586.57, 781.93, 913.80]


all_data = [458.12, 339.93, 425.89, 506.96, 371.18,
            576.80, 416.61, 716.00, 643.71, 583.64,
            535.29, 781.44, 737.00, 578.75, 959.22]
all_data =[481.56, 325.76, 440.54, 498.17, 376.07,
           591.94, 390.72, 716.00, 628.08, 604.15,
           525.52, 756.04, 756.53, 564.10, 961.66]
all_data = [490.84, 309.65, 449.82, 485.96, 383.88,
            573.38, 405.37, 693.04, 609.52, 620.76,
            511.84, 753.11, 740.90, 533.33, 951.89]

#sapi 1
all_data = [29.30, 219.78, 103.05, 119.17, 118.19,
            41.03, 222.22, 256.41, 147.99, 215.87,
            305.74, 278.39, 306.23, 333.09, 474.24]
all_data = [28.82, 196.34, 99.63, 107.94, 113.31,
            11.23, 206.10, 241.27, 151.89, 182.66,
            281.81, 265.69, 284.25, 310.62, 427.84]
all_data = [18.07, 215.38, 84.98, 124.05, 134.80,
            10.26, 209.52, 231.99, 161.17, 154.33,
            288.64, 255.92, 244.69, 304.27, 452.26]
#sapi 2
all_data = []


layer1_data = all_data[:5]   # 5 measurements
layer2_data = all_data[5:9]  # 4 measurements
layer3_data = all_data[9:12] # 3 measurements
layer4_data = all_data[12:14] # 2 measurements
layer5_data = all_data[14:]   # 1 measurement

# Combine all layers
wenner_data = [layer1_data, layer2_data, layer3_data, layer4_data, layer5_data]

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
im = ax.imshow(grid_z, cmap='jet', norm=LogNorm(vmin=np.min(all_data), vmax=np.max(all_data)),
               extent=[0, max_width, height, 0], aspect='auto')

# Customize the plot
ax.set_title('Wenner Array Resistivity Profile (5 Layers)', fontsize=16)
ax.set_xlabel('Distance', fontsize=12)
ax.set_ylabel('Depth', fontsize=12)

# Remove x-axis ticks
ax.set_xticks([])

# Set y-axis ticks and labels
ax.set_yticks(np.arange(len(wenner_data)))
ax.set_yticklabels([f'a={i+2}' for i in range(len(wenner_data))])

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
