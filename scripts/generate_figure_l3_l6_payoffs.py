import pandas as pd
import sys
import matplotlib.pyplot as plt

plt.rcParams["font.family"] = "Arial"

if __name__ == "__main__":
    base = sys.argv[1]
    df = pd.read_csv(base + "L6_L3_payoff_difference.csv")

    groups = list(df.groupby(['cost']))
    payoff_differences = {(0.2, 1): 0, (0.6, 1): 0, (0.2, 2): 0, (0.6, 2): 0}

    for i, cost_group in groups:
        norms_group = cost_group.groupby("order")
        for j, norm in norms_group:
            difference = norm['selfpay'].iloc[0] - norm['mutantpay'].mean()

            payoff_differences[(i[0], j)] += difference

    fig, axes = plt.subplot_mosaic("""A.B""",
                                sharey=True, sharex=False, figsize=(6.5, 3),
                                gridspec_kw={'width_ratios': [1, .1, 1]})


    axes["A"].bar(1, payoff_differences[(0.2, 1)], color="forestgreen", edgecolor='black', linewidth=1,);
    axes["A"].bar(0, payoff_differences[(0.2, 2)], color="cornflowerblue", edgecolor='black', linewidth=1,);


    axes["B"].bar(1, payoff_differences[(0.6, 1)], color="forestgreen", edgecolor='black', linewidth=1,);
    axes["B"].bar(0, payoff_differences[(0.6, 2)], color="cornflowerblue", edgecolor='black', linewidth=1,);

    axes["A"].set_ylabel("Payoff difference\n" + r"$\pi_\text{res} - \langle \pi_\text{mut} \rangle$");

    axes["A"].set_title("Low cooperation cost ($c=0.2$)", y=1.1)
    axes["B"].set_title("High cooperation cost ($c=0.6$)", y=1.1)

    for i, letter in enumerate("AB"):
        axes[letter].spines[['right', 'top']].set_visible(False)
        axes[letter].set_xticks([0, 1])
        axes[letter].set_xticklabels(["L6", "L3"])

    fig.savefig(f"Figures/L3_&_L6_payoff_diff.pdf", bbox_inches='tight')