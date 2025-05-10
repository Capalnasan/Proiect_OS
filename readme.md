# 🗺️ Treasure Hunt Game System – OS Project

This is a Linux-based system programming project simulating a **Treasure Hunt Game Manager**, composed of multiple C programs that interact through files and directories. It handles adding, listing, and managing treasures with logging and directory manipulation.

## 📁 Project Structure

- `monitor.c` – Monitors logs from treasure hunts and displays operations.
- `treasure.c` – Main treasure manager. Handles treasure creation, storage, and clues.
- `treasure_hub.c` – Central coordinator for creating treasure hunts and managing operations.

---

## ⚙️ Compilation

To compile all components, use:

```bash
gcc -o monitor monitor.c
gcc -o treasure treasure.c
gcc -o treasure_hub treasure_hub.c
```

Make sure you have `gcc` installed:

```bash
sudo apt update
sudo apt install build-essential
```

---

## ▶️ How to Run

1. **Create a new treasure hunt:**

```bash
./treasure_hub createhunt <hunt_name>
```

2. **Add a treasure to a hunt:**

```bash
./treasure add <hunt_name> <id> <username> <latitude> <longitude> <clue> <value>
```

3. **List all treasures:**

```bash
./treasure list <hunt_name>
```

4. **Remove a treasure:**

```bash
./treasure remove <hunt_name> <id>
```

5. **Start monitoring operations (in a separate terminal):**

```bash
./monitor <hunt_name>
```

---

## 📜 Command List

### `treasure_hub`

- `createhunt <hunt_name>`: Initializes the folder structure for a new hunt.

### `treasure`

- `add <hunt_name> <id> <username> <lat> <long> <clue> <value>`: Adds a treasure.
- `list <hunt_name>`: Lists all treasures in the hunt.
- `remove <hunt_name> <id>`: Deletes the treasure with the given ID.

### `monitor`

- `<hunt_name>`: Continuously monitors and prints logged operations from that hunt.

---

## 📝 Logs

Each hunt has a `logged_hunt` file storing every operation (timestamped), created in the hunt directory.

Example log entry:

```
[1715203200] Added treasure with ID 101 by user Alex
```

---

## 🧪 Example Session

```bash
./treasure_hub createhunt island_mystery
./treasure add island_mystery 1 user1 45.23 23.55 "Under the palm tree" 100
./treasure list island_mystery
./monitor island_mystery
```

---

## 🧑‍💻 Author

Project developed as part of the Operating Systems course.
