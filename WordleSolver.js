let wasmReady = false;

Module.onRuntimeInitialized = () => {
    document.getElementById("suggestion").textContent = "Building solver...";
    setTimeout(() => {
        Module.initialize("words_1.txt");
        wasmReady = true;
        updateUI();
    }, 50);
}

function updateUI() {

    const suggestion = Module.getSuggestion();
    const remaining = Module.getRemainingCount();

    document.getElementById("suggestion").textContent = suggestion;
    document.getElementById("remaining").textContent = remaining;
}
 
document.addEventListener("DOMContentLoaded", () => {

    const submitBtn = document.getElementById("submitBtn");
    const resetBtn = document.getElementById("resetBtn");

    submitBtn.addEventListener("click", () => {

        if (!wasmReady) return;

        const tiles = document.querySelectorAll(".tile");

        let guess = "";
        let pattern = "";

        tiles.forEach(tile => {

            guess += tile.textContent.toLowerCase();

            if (tile.classList.contains("green")) {
                pattern += "G";
            }
            else if (tile.classList.contains("yellow")) {
                pattern += "Y";
            }
            else {
                pattern += "B";
            }
        });

        if (guess.length !== 5 || /[^a-z]/.test(guess)) {
            alert("Enter a 5-letter word.");
            return;
        }

        const result = Module.applyGuess(guess, pattern);

        let message = "";

        switch (result) {

            case "solved":
                message = "Your Wordle has been solved.";
                break;

            case "no_candidates":
                message = "No candidates left.";
                break;

            case "invalid":
                message = "Word not found.";
                break;

            case "ongoing":
                message = "Add your next guess.";
                break;

            default:
                if (result.startsWith("answer:")) {
                    message =
                        "Your answer is: " +
                        result.substring(7);
                }
        }

        document.getElementById("status").textContent = message;

        updateUI();
        tiles.forEach(tile => {

            tile.textContent = "";

            tile.classList.remove("yellow");
            tile.classList.remove("green");

            tile.classList.add("grey");
        });
    });
    resetBtn.addEventListener("click", () => {

        if (!wasmReady) return;

        Module.reset();

        const tiles = document.querySelectorAll(".tile");

        tiles.forEach(tile => {

            tile.textContent = "";

            tile.classList.remove("yellow");
            tile.classList.remove("green");

            tile.classList.add("grey");
        });

        document.getElementById("status").textContent =
            "Add your guess.";

        updateUI();

        document.querySelector(".tile").focus();
    });
});

document.addEventListener("DOMContentLoaded", () => {

    const tiles = document.querySelectorAll(".tile");

    tiles.forEach((tile, index) => {

        tile.addEventListener("input", () => {

            tile.textContent =
                tile.textContent
                    .replace(/[^a-zA-Z]/g, "")
                    .toUpperCase()
                    .slice(0, 1);

            if (tile.textContent && index < tiles.length - 1) {
                tiles[index + 1].focus();
            }
        });

        tile.addEventListener("click", () => {

            if (tile.classList.contains("grey")) {

                tile.classList.remove("grey");
                tile.classList.add("yellow");

            } else if (tile.classList.contains("yellow")) {

                tile.classList.remove("yellow");
                tile.classList.add("green");

            } else {

                tile.classList.remove("green");
                tile.classList.add("grey");
            }
        });
    });
});