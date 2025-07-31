# Reconstructing MBP-10 from MBO

This project reconstructs an MBP-10 order book from MBO data using C++. It processes `mbo.csv` and writes the top-10 price levels to `outtest.csv`.

## 🔧 Build & Run

```bash
make
./reconstruction_blockhouse mbo.csv
```

Output: `outtest.csv`

#############    IMP NOTE  ##################

While implementing the mbp10 from the mbo I have seen that data in the given mbp10 does not corressponds to the each mbo order ids. Therefore, I have implemented entries in mbp10 for each order id whether it is add/cancel for ask/bid. There might be some entries missing the given mbp10 file or the requirement is not clear to me. I have raised an email for this but didn't get revert.
I have give the submission for the problem understanding that I have, if this was clear then I built for this requirement also. 
Thanks




#############################################

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
