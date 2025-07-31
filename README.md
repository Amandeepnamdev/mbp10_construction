# Reconstructing MBP-10 from MBO

This project reconstructs an MBP-10 order book from MBO data using C++. It processes `mbo.csv` and writes the top-10 price levels to `outtest.csv`.

## 🔧 Build & Run

```bash
make
./reconstruction_blockhouse mbo.csv
```

Output: `outtest.csv`

## ⚙️ Optimization Highlights

- **Efficient Maps**: Used `std::map` with `greater<>` for bid, natural order for ask.
- **Trade Coalescing**: For `T→F→C` sequences, ignored `F` and `C`, applied `T` as a synthetic cancel on the opposite side using `C`'s size.
- **I/O Efficient**: Output only on book change to reduce disk writes.
- **Line-Based Processing**: Avoided loading entire file into memory.

## 📌 Notes

- Skip rows with action `R`.
- Skip trades with side `N`.
- Only top 10 levels are output.
- Format matches provided `mbp.csv`.

## 📁 Files

- `mbp10_construction.cpp` – Core logic
- `Makefile` – Build instructions
- `README.md` – Project docs

## ✅ Sample Usage

```bash
./reconstruction_blockhouse mbo.csv
```

Produces an MBP-10 snapshot after each book update in `outtest.csv`.
