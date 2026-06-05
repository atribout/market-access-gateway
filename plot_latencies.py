import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Charger les données
df = pd.read_csv("build/latencies.csv")

# Filtrer les outliers extrêmes (ex: interruptions OS résiduelles) pour centrer la cloche
q99_9 = df['Latency_ns'].quantile(0.999)
df_filtered = df[df['Latency_ns'] < q99_9]

# Configuration du style "Dark Mode"
plt.figure(figsize=(10, 6), facecolor='#1e1e1e')
sns.set_theme(style="darkgrid", rc={"axes.facecolor": "#2d2d2d", "figure.facecolor":"#1e1e1e", "grid.color": "#404040"})

# Tracer l'histogramme
ax = sns.histplot(df_filtered['Latency_ns'], bins=100, kde=True, color='#00ffff', edgecolor='black')

# Ajouter des lignes de percentiles
p50 = df['Latency_ns'].quantile(0.50)
p99 = df['Latency_ns'].quantile(0.99)

plt.axvline(p50, color='#ff3333', linestyle='dashed', linewidth=2, label=f'p50: {p50:.2f} ns')
plt.axvline(p99, color='#ff9933', linestyle='dashed', linewidth=2, label=f'p99: {p99:.2f} ns')

# Customisation des textes
plt.title('Gateway Processing Latency Distribution (O(1) Passive LOB)', fontsize=14, fontweight='bold', color='white', pad=15)
plt.xlabel('Latency (Nanoseconds)', fontsize=12, color='white')
plt.ylabel('Frequency (Message Count)', fontsize=12, color='white')
ax.tick_params(colors='white')
plt.legend(facecolor='#2d2d2d', edgecolor='white', labelcolor='white')

# Sauvegarder l'image
plt.tight_layout()
plt.savefig('latency_histogram.png', dpi=300, facecolor='#1e1e1e')
print("Graphique généré avec succès : latency_histogram.png")