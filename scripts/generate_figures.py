import sys
import subprocess

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python run_all_figures.py <base_data_path>")
        sys.exit(1)

    base = sys.argv[1]

    # Creates Figures 1 and 4
    subprocess.run(["python", "scripts/generate_figure_l8_errors.py", base], check=True)

    # Creates Figure 2
    subprocess.run(["python", "scripts/generate_figure_equalizers.py", base], check=True)

    # Creates Figure 3
    subprocess.run(["python", "scripts/generate_figure_l3_l6_payoffs.py", base], check=True)
