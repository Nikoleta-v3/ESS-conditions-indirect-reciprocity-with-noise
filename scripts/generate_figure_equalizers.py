import pandas as pd
import sys
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D

plt.rcParams["font.family"] = "Arial"

if __name__ == "__main__":
    base = sys.argv[1]
    df = pd.read_csv(base + "equalizers_payoffs.csv")

    SIDs = [15, 5, 10, 0]
    strategies = [(1, 1, 1, 1), (1, 0, 1, 0), (0, 1, 0, 1), (0, 0, 0, 0)]
    colors = ["forestgreen", "cornflowerblue", "forestgreen", "forestgreen"]

    handles = [
        Line2D([0], [0], color="cornflowerblue", lw=4, label="Resident"),
        Line2D([0], [0], color="forestgreen", lw=4, label="Mutant"),
    ]

    fig, axes = plt.subplot_mosaic(
        """AB""",
        sharey=True,
        sharex=True,
        figsize=(10, 3),
        gridspec_kw={"hspace": 0.35},
    )

    # Plot for Generous Scoring
    for i, s in enumerate(SIDs):
        payoff = df[(df["order"] == 1) & (df["SID"] == s)].iloc[0]["mutantpay"]
        axes["A"].bar(
            i,
            payoff,
            color=colors[i],
            width=0.5,
            edgecolor="black",
            linewidth=1,
        )

    # Plot for Cautious Scoring
    for i, s in enumerate(SIDs):
        payoff = df[(df["order"] == 2) & (df["SID"] == s)].iloc[0]["mutantpay"]
        axes["B"].bar(
            i,
            payoff,
            color=colors[i],
            width=0.5,
            edgecolor="black",
            linewidth=1,
        )

    axes["A"].set_ylim(0, 1)
    axes["A"].set_xticks(range(4))
    axes["A"].set_xticklabels(["ALLC", "DISC", "ADISC", "ALLD"])
    axes["A"].set_ylabel(r"Payoff ($\pi$)")
    axes["A"].set_title("Generous Scoring")
    axes["B"].set_title("Cautious Scoring")

    for letter in "AB":
        axes[letter].spines[["right", "top"]].set_visible(False)

    axes["B"].legend(handles=handles)

    fig.savefig("Figures/Equalizers.pdf", bbox_inches="tight")
