import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
import sympy as sym
import sys

plt.rcParams["font.family"] = "Arial"
fontsizesmall = 11
fontsizelarge = 13
fontsizehuge = 18
fontsizetiny = 9
cmap = mpl.colormaps['coolwarm_r']
base_color = 'white'

b, c, mu, me, edc = sym.symbols("b, c, mu, mu_e, e_{dc}")
threshold_expr = sym.solve((1 / ((1 - 2 * mu) * (1 - me) * (1 - edc)) - b / c), me)[0].factor()
f = sym.lambdify((b, c, mu, edc), threshold_expr)

benefit = 1
cost = 0.8

def calc_ess_condition(X, Y, mu, me, e, DU, c):

    RC = (1 - mu) * X + mu * (1 - X)
    RD = (1 - mu) * Y + mu * (1 - Y)

    RD = (1 - e) * RD + e * RC

    RC = (1 - me) * RC + me * RD

    lhs = (RC - RD) * DU
    
    return (lhs - c)

def calc_L1_DU(b, c, h, mu, mue, e):
    num = b * h + c * (1 - h)
    den = 1 - (1 - h) * (1 - 2 * mu) * mue * (1 - e)
    return num / den

def calc_L2_DU(b, c, h, mu, mue, e):
    num = b * h + c * (1 - h)
    den = 1 - (1 - h) * (1 - 2 * mu) * (mue - e - mue * e)
    return num / den

def calc_L3_DU(b, c, h, mu, mue, e):
    return b

def calc_L4_DU(b, c, h, mu, mue, e):
    num = b
    den = 1 - (1 - h) * (1 - 2 * mu) * e
    return num / den

def calc_L5_DU(b, c, h, mu, mue, e):
    num = b
    den = 1 + (1 - h) * (1 - 2 * mu) * e
    return num / den

def calc_L6_DU(b, c, h, mu, mue, e):
    return b

def calc_L7_DU(b, c, h, mu, mue, e):
    num = b
    den = 1 - (1 - h) * (1 - 2 * mu)
    return num / den

def calc_L8_DU(b, c, h, mu, mue, e):
    num = b
    den = 1 - (1 - h) * (1 - 2 * mu) * (1 - e)
    return num / den


def get_calc_function(n):
    functions = {
        1: calc_L1_DU,
        2: calc_L2_DU,
        3: calc_L3_DU,
        4: calc_L4_DU,
        5: calc_L5_DU,
        6: calc_L6_DU,
        7: calc_L7_DU,
        8: calc_L8_DU
    }
    return functions.get(n, None)

if __name__ == "__main__":
    base = sys.argv[1]
    df = pd.read_csv(base + "leading_eight_ESS_with_errors.csv")

    # L3 and L6
    fig, axes = plt.subplot_mosaic("""ABCDE
                                    FGHIJ""",
                                sharey=True, sharex=True, figsize=(15, 6),
                                gridspec_kw={"hspace": 0.35})

    assessment_errors = [0.002, 0.02, 0.04, 0.06, 0.08]

    for letter, assessment_error in zip("ABCDE", assessment_errors):
        data_check = np.zeros((len(df['perception_error'].unique()), len(df['mu_e'].unique())))
        sub = df[(df['ID'] == 3002) & (df['assessment_error'] == assessment_error)]

        for i, perception in enumerate(df['perception_error'].unique()):
            for j, mu_e in enumerate(df['mu_e'].unique()):
                data_check[i, j] = sub[(sub['perception_error'] == perception) & (sub['mu_e'] == mu_e)]['isNash'].iloc[0]

        try:
            points = [(f(benefit, cost, assessment_error, p), p) for p in df['perception_error'].unique()]
            points = [((50 * x) / 0.1, (50 * y) / 0.1) for x, y in points if 0 <= x <= 0.1]
            axes[letter].plot(*zip(*points), color=base_color, linewidth=2.5)

            if letter in "BCD":
                x, y = zip(*points)
                x_pos = np.median(x)
                y_pos = np.median(y)
                axes[letter].text(x_pos - 15, y_pos, 'Theoretical \n prediction',
                                rotation=-45, color=base_color,
                                fontsize=fontsizetiny, fontweight='bold',
                                rotation_mode='anchor')
            if letter == "A":
                axes[letter].text(20, 23, "ESS", color=base_color,
                                fontsize=fontsizelarge, fontweight='bold')
            if letter == "E":
                axes[letter].text(16, 23, "Not ESS", color=base_color,
                                fontsize=fontsizelarge, fontweight='bold')
        except ValueError:
            pass

        axes[letter].set_title(r'Assessment error ($\mu$) ' + f'{assessment_error}')
        axes[letter].imshow(data_check, origin='lower', cmap=cmap, vmin=0, vmax=1)

    for letter, assessment_error in zip("FGHIJ", assessment_errors):
        data_check = np.zeros((len(df['perception_error'].unique()), len(df['mu_e'].unique())))
        sub = df[(df['ID'] == 2458) & (df['assessment_error'] == assessment_error)]

        for i, perception in enumerate(df['perception_error'].unique()):
            for j, mu_e in enumerate(df['mu_e'].unique()):
                data_check[i, j] = sub[(sub['perception_error'] == perception) & (sub['mu_e'] == mu_e)]['isNash'].iloc[0]

        try:
            points = [(f(benefit, cost, assessment_error, p), p) for p in df['perception_error'].unique()]
            points = [((50 * x) / 0.1, (50 * y) / 0.1) for x, y in points if 0 <= x <= 0.1]
            axes[letter].plot(*zip(*points), color=base_color, linewidth=2.5)

            if letter in "GHI":
                x, y = zip(*points)
                x_pos = np.median(x)
                y_pos = np.median(y)
                axes[letter].text(x_pos - 15, y_pos + 1.5, 'Theoretical \n prediction',
                                rotation=-45, color=base_color,
                                fontsize=fontsizetiny, fontweight='bold',
                                rotation_mode='anchor')
            if letter == "F":
                axes[letter].text(20, 23, "ESS", color=base_color,
                                fontsize=fontsizelarge, fontweight='bold')
            if letter == "J":
                axes[letter].text(15, 23, "Not ESS", color=base_color,
                                fontsize=fontsizelarge, fontweight='bold')
        except ValueError:
            pass

        axes[letter].imshow(data_check, origin='lower', cmap=cmap, vmin=0, vmax=1)

    for letter in "AF":
        axes[letter].set_ylabel(r"Perception error ($e_{DC}$)")
        axes[letter].set_xticks(range(0, 51, 10))
        axes[letter].set_yticks(range(0, 51, 10))
        axes[letter].set_yticklabels([df['perception_error'].unique()[i].round(2) for i in range(0, 51, 10)])
        axes[letter].set_xticklabels([df['mu_e'].unique()[i].round(2) for i in range(0, 51, 10)])

    axes["A"].text(-27, 22, "L3", fontsize=fontsizehuge, fontweight='bold')
    axes["F"].text(-27, 22, "L6", fontsize=fontsizehuge, fontweight='bold')

    fig.savefig("Figures/ErrorsL3L6.pdf", bbox_inches='tight')

    # All leading eight
    fig, axes = plt.subplots(ncols=5, nrows=8, sharey=True, sharex=True, figsize=(15, 24), gridspec_kw={"hspace": 0.35})

    labels = [f'L{i}' for i in range(1, 9)]
    ypos = [-1, 3, -1, -1, 0, -1, -1, -1]
    xpos = [15, 17, 15, 15, 15, 15, 15, 15]

    for irow, (label, ID) in enumerate(zip(labels, df['ID'].unique())):
        for icol, assessment_error in zip(range(5), [0.002, 0.02, 0.04, 0.06, 0.08]):
            data_check = np.zeros((len(df['perception_error'].unique()), len(df['mu_e'].unique())))
            sub = df[(df['ID'] == ID) & (df['assessment_error'] == assessment_error)]

            for i, perception in enumerate(df['perception_error'].unique()):
                for j, mu_e in enumerate(df['mu_e'].unique()):
                    isNash = sub[(sub['perception_error'] == perception) & (sub['mu_e'] == mu_e)]['isNash'].iloc[0]
                    data_check[i, j] = isNash

            if label == 'L1':
                axes[irow, icol].set_title(r'Assessment error ($\mu$) ' + f'{assessment_error}', y=1.1)

            axes[irow, icol].imshow(data_check, origin='lower', cmap='coolwarm_r', vmin=0, vmax=1)

            if label == 'L8':
                axes[irow, icol].set_xlabel(r"Implementation error ($\mu_e$)")

            axes[irow, 0].text(-27, 22, label, fontsize=fontsizehuge, fontweight='bold')

            if icol == 0:
                axes[irow, icol].text(20, 23, "ESS", color="white", fontsize=fontsizelarge, fontweight='bold', rotation_mode='anchor')
            if (icol == 4) and (irow < 6):
                axes[irow, icol].text(15, 23, "Not ESS", color="white", fontsize=fontsizelarge, fontweight='bold', rotation_mode='anchor')

            DU_funct = get_calc_function(sub['order'].iloc[0])
            points = []

            for _, row in sub.iterrows():
                h, e, mue = row['h'], row['perception_error'], row['mu_e']
                DU = DU_funct(benefit, cost, h, assessment_error, mue, e)
                diff = calc_ess_condition(1, 0, assessment_error, mue, e, DU, cost)

                if abs(diff) < 0.0008:
                    points.append((mue * 50 / 0.1, e * 50 / 0.1))

            try:
                x_vals, y_vals = zip(*points)
                x1 = np.array(x_vals)
                y1 = np.array(y_vals)
                m, b1 = np.polyfit(x1, y1, 1)
                line_y = m * x1 + b1
                axes[irow, icol].plot(x1, line_y, color='white', linewidth=2)

                if icol in [1, 2, 3] and irow not in [6, 7]:
                    x_pos = np.median(x1)
                    y_pos = np.median(y1)
                    offset_x = xpos[irow]
                    offset_y = ypos[irow]
                    if icol == 2 and irow == 1:
                        axes[irow, icol].text(x_pos - offset_x, y_pos - 1, 'Theoretical \n prediction',
                                            rotation=-45, color="white", fontsize=fontsizetiny,
                                            fontweight='bold', rotation_mode='anchor')
                    else:
                        axes[irow, icol].text(x_pos - offset_x, y_pos + offset_y, 'Theoretical \n prediction',
                                            rotation=-45, color="white", fontsize=fontsizetiny,
                                            fontweight='bold', rotation_mode='anchor')
            except ValueError:
                pass

    for i in range(8):
        axes[i, 0].set_ylabel(r"Perception error ($e_{DC}$)")
        axes[i, 0].set_xticks(range(0, 51, 10))
        axes[i, 0].set_yticks(range(0, 51, 10))
        axes[i, 0].set_yticklabels([df['perception_error'].unique()[j].round(2) for j in range(0, 51, 10)])
        axes[i, 0].set_xticklabels([df['mu_e'].unique()[j].round(2) for j in range(0, 51, 10)])

    fig.savefig("Figures/ErrorsLeadingEight.pdf", bbox_inches='tight')
