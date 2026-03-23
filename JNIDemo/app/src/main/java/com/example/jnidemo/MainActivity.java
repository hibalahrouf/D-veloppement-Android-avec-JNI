package com.example.jnidemo;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {
    public native String detectForbiddenChars(String inputString);
    public native int[] multiplyMatrices(int[] matA, int[] matB, int size);
    public native String greetFromNative();
    public native int computeFactorial(int inputVal);
    public native String flipString(String sourceString);
    public native int accumulateArray(int[] intData);

    static {
        System.loadLibrary("jnidemo");
    }

    // Calcul factoriel côté Java pur
    private long javaFactorial(int inputVal) {
        if (inputVal < 0) return -1;
        long result = 1;
        for (int step = 1; step <= inputVal; step++) {
            result *= step;
        }
        return result;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView tvHello     = findViewById(R.id.tvHello);
        TextView tvFact      = findViewById(R.id.tvFact);
        TextView tvReverse   = findViewById(R.id.tvReverse);
        TextView tvArray     = findViewById(R.id.tvArray);
        TextView tvBenchmark = findViewById(R.id.tvBenchmark);

        // Salutation native
        tvHello.setText(greetFromNative());

        // Tests factoriel
        tvFact.setText(
                "10! = " + computeFactorial(10) + "\n" +
                        "factorial(-5) = " + computeFactorial(-5) + "\n" +
                        "factorial(20) = " + computeFactorial(20)
        );

        // Tests inversion
        tvReverse.setText(
                "Inverse : " + flipString("JNI c'est puissant !") + "\n" +
                        "Chaine vide : \"" + flipString("") + "\""
        );

        // Tests tableau
        tvArray.setText(
                "Total [10,20,30,40,50] = " + accumulateArray(new int[]{10, 20, 30, 40, 50}) + "\n" +
                        "Tableau vide = " + accumulateArray(new int[]{})
        );

        // ── BENCHMARK ──────────────────────────────────

        int benchInput = 15;
        int iterations = 100000; // 100 000 répétitions pour mesurer correctement

        // Mesure Java
        long javaStart = System.nanoTime();
        for (int i = 0; i < iterations; i++) {
            javaFactorial(benchInput);
        }
        long javaDuration = System.nanoTime() - javaStart;

        // Mesure C++ via JNI
        long nativeStart = System.nanoTime();
        for (int i = 0; i < iterations; i++) {
            computeFactorial(benchInput);
        }
        long nativeDuration = System.nanoTime() - nativeStart;

        // Calcul moyen par appel
        long javaAvg  = javaDuration  / iterations;
        long nativeAvg = nativeDuration / iterations;

        String winner = javaAvg < nativeAvg ? "Java est plus rapide !" : "C++ est plus rapide !";

        tvBenchmark.setText(
                "── Benchmark " + iterations + " x factorial(" + benchInput + ") ──\n" +
                        "Java   total : " + javaDuration   + " ns\n" +
                        "Java   moy   : " + javaAvg        + " ns/appel\n" +
                        "C++    total : " + nativeDuration  + " ns\n" +
                        "C++    moy   : " + nativeAvg       + " ns/appel\n" +
                        "→ " + winner
        );
        // ── MATRICE ──────────────────────────────────
        TextView tvMatrix = findViewById(R.id.tvMatrix);

// Matrice A :        Matrice B :
// | 1  2 |           | 5  6 |
// | 3  4 |           | 7  8 |
        int[] matA = {1, 2, 3, 4};
        int[] matB = {5, 6, 7, 8};
        int matSize = 2;

        int[] matResult = multiplyMatrices(matA, matB, matSize);

        tvMatrix.setText(
                "── Multiplication matricielle 2x2 ──\n" +
                        "A = [1,2 | 3,4]\n" +
                        "B = [5,6 | 7,8]\n" +
                        "A×B = [" + matResult[0] + "," + matResult[1] +
                        " | " + matResult[2] + "," + matResult[3] + "]\n" +
                        "Attendu : [19,22 | 43,50]"
        );
        // ── DETECTION CARACTERES INTERDITS ───────────
        TextView tvDetect = findViewById(R.id.tvDetect);

        String testClean   = "HelloWorld";
        String testDirty1  = "user@domain.com";
        String testDirty2  = "prix$100#promo";
        String testDirty3  = "normal texte";

        tvDetect.setText(
                "── Detection caracteres interdits ──\n" +
                        "\"" + testClean  + "\" → " + detectForbiddenChars(testClean)  + "\n" +
                        "\"" + testDirty1 + "\" → " + detectForbiddenChars(testDirty1) + "\n" +
                        "\"" + testDirty2 + "\" → " + detectForbiddenChars(testDirty2) + "\n" +
                        "\"" + testDirty3 + "\" → " + detectForbiddenChars(testDirty3)
        );

    }

}