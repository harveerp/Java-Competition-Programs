import java.math.BigInteger;
import java.util.Scanner;

/**
 * Created with IntelliJ IDEA.
 * User: harveerparmar
 * Date: 2013-03-13
 * Time: 3:02 PM
 * To change this template use File | Settings | File Templates.
 */
public class Main implements Runnable {

    public static void main(String[] args){
        Main myWork = new Main();
        myWork.run();
    }

    public void run(){
        new myStuff().run();
    }
}

class myStuff implements Runnable{
    BigInteger n;

    Scanner scanner = new Scanner(System.in);
    Scanner lineReader;
    int testCases;
    StringBuilder output = new StringBuilder();

    public void run(){


        lineReader = new Scanner(scanner.nextLine());
        testCases = lineReader.nextInt();

        for(int i = 0; i < testCases; i++){

            lineReader = new Scanner(scanner.nextLine());
            n = lineReader.nextBigInteger();
            output.append(pieces(n));

            if(i != testCases - 1){
                output.append("\n");
            }

        }
        System.out.println(output.toString());

    }

    private static BigInteger pieces(BigInteger n){
        BigInteger piece;
        BigInteger piece1;
        BigInteger piece2;
        BigInteger piece3;
        BigInteger piece4;
        BigInteger piece5;
        BigInteger combine;



        piece =  n.pow(4);
        piece1 = n.pow(3).multiply(BigInteger.valueOf(6));
        piece2 = n.pow(2).multiply(BigInteger.valueOf(23));
        piece3 = n.pow(1).multiply(BigInteger.valueOf(18));
        piece4 = BigInteger.valueOf(24);
        piece5 = BigInteger.valueOf(24);

        combine = BigInteger.ZERO;
        combine = combine.add(piece);
        combine = combine.subtract(piece1);
        combine = combine.add(piece2);
        combine = combine.subtract(piece3);
        combine = combine.add(piece4);
        combine = combine.divide(piece5);



        return combine;

    }
}
