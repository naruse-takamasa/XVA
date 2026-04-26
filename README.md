# XVA Calculator — C++17

IRS向けXVA（CVA/DVA/FVA/MVA）計算エンジン。  
Hull-White 1ファクターモデル + マルチスレッドモンテカルロ。

---

## アーキテクチャ

```
xva_project/
├── include/
│   ├── Common.hpp                  # 共通型: YieldCurve, CreditCurve, SimulationResult, XVAResult
│   ├── models/
│   │   └── HullWhiteModel.hpp      # HW1F短期金利モデル
│   ├── instruments/
│   │   └── InterestRateSwap.hpp    # バニラIRS（ペイヤー/レシーバー）
│   ├── simulation/
│   │   ├── MonteCarloEngine.hpp    # マルチスレッドMCエンジン
│   │   └── ExposureCalculator.hpp  # EE/ENE/PFE/IMプロファイル
│   ├── xva/
│   │   ├── CVACalculator.hpp       # CVA = (1-R)·Σ EE·ΔQ·D
│   │   ├── DVACalculator.hpp       # DVA = (1-R)·Σ |ENE|·ΔQ_own·D
│   │   ├── FVACalculator.hpp       # FCA + FBA
│   │   ├── MVACalculator.hpp       # IM funding cost
│   │   └── XVAAggregator.hpp       # 統合計算・レポート
│   └── utils/
│       ├── MathUtils.hpp           # normCDF, interp, trapz
│       ├── DateUtils.hpp           # TimeGrid生成
│       └── Logger.hpp              # ログ
└── src/ tests/                     # 実装・テスト
```

外部依存なし — C++17標準ライブラリのみ（テストフレームワーク: [GoogleTest](https://github.com/google/googletest) を CMake FetchContent で自動取得）。

---

## 数式概要

### Hull-White 1F
```
dr(t) = [θ(t) - a·r(t)]dt + σ·dW(t)
P(t,T) = exp(A(t,T) - B(t,T)·r(t))
B(t,T) = (1 - e^{-a(T-t)}) / a
```

### CVA
```
CVA = -(1-R_c) · Σ_i  EE(t_i) · [Q_c(t_{i-1}) - Q_c(t_i)] · D(t_i)
```

### DVA
```
DVA = (1-R_o) · Σ_i  |ENE(t_i)| · [Q_o(t_{i-1}) - Q_o(t_i)] · D(t_i)
```

### FVA
```
FCA = -s_borrow · Σ_i  EE(t_i)   · D(t_i) · Δt_i
FBA = +s_lend   · Σ_i |ENE(t_i)| · D(t_i) · Δt_i
FVA = FCA + FBA
```

### MVA
```
MVA = -s_im · Σ_i  IM(t_i) · D(t_i) · Δt_i
IM  ≈ 1.4 · σ_exposure · √(10/252)    (SIMM proxy)
```

---

## ビルド

```bash
# リリースビルド
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# デバッグビルド
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

## 実行

```bash
./build/xva_calculator
```

## テスト

GoogleTest を使用。

```bash
# 全テスト実行
cd build && ctest --output-on-failure

# 個別テスト実行
./build/test_irs
./build/test_hw_model
./build/test_exposure
./build/test_xva
```

## サニタイザ

```bash
# AddressSanitizer + UndefinedBehaviorSanitizer（要 clang++）
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DSANITIZE=ON -DCMAKE_CXX_COMPILER=clang++
cmake --build build --parallel
cd build && ctest --output-on-failure

# ThreadSanitizer — MCエンジンのデータレース検出（SANITIZE と排他）
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DTSAN=ON -DCMAKE_CXX_COMPILER=clang++
cmake --build build --parallel
cd build && TSAN_OPTIONS=halt_on_error=1 ctest --output-on-failure
```

---

## CI/CD（GitHub Actions）

| ジョブ | 内容 |
|---|---|
| `lint` | clang-format チェック + clang-tidy 静的解析 |
| `build-and-test` | Release / Debug マトリクスビルド + テスト |
| `sanitize` | ASan + UBSan ビルド + テスト |
| `tsan` | ThreadSanitizer ビルド + テスト |
| `release` | タグ `v*` 時に CPack で TGZ パッケージ化し GitHub Releases へアップロード |

---

## デフォルトパラメータ（main.cpp）

| パラメータ | 値 |
|---|---|
| スワップ満期 | 5年 |
| 想定元本 | 1,000,000 |
| 固定金利 | パーレート（ATM） |
| HW mean-reversion `a` | 0.05 |
| HW volatility `σ` | 0.01 |
| MCパス数 | 5,000 |
| タイムステップ | 月次（60ステップ） |
| カウンターパーティ hazard rate | 100 bps |
| 自社 hazard rate | 50 bps |
| Recovery rate | 40% |
| Funding borrow spread | 50 bps |
| IM funding spread | 30 bps |

---

## 拡張ポイント

- **複数通貨スワップ**: `instruments/` に `CrossCurrencySwap` を追加
- **Sobol列**: `MCConfig::useQuasiMC = true` でQMCに切替予定
- **ISDA SIMM**: `ExposureCalculator` のIM計算を本格実装
- **Adjoint AD**: XVAギリシャのAD微分（xAD/Tapasco対応）
- **GPU並列化**: CUDA対応MCエンジン（`simulation/CudaMCEngine`）
