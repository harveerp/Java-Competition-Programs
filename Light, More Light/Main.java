import java.util.Scanner;

/**
 * Created with IntelliJ IDEA.
 * User: harveerparmar
 * Date: 2013-02-27
 * Time: 4:10 PM
 * To change this template use File | Settings | File Templates.
 */
class Main implements Runnable {



        public static void main(String[] args){

            Main myWork = new Main();
            myWork.run();
        }
        public void run(){
            new myStuff().run();
        }
    }

    class myStuff implements Runnable{

        private Scanner scanner = new Scanner(System.in);
        private StringBuilder output = new StringBuilder();
        long number;

        public void run(){

            Scanner lineReader;


            lineReader = new Scanner(scanner.nextLine());
            number = lineReader.nextLong();

            while(number != 0){


                long squareRoot = (long) (Math.floor(Math.sqrt(number) + 0.5));

                if(squareRoot * squareRoot == number)
                    output.append("yes");
                else
                    output.append("no");

                lineReader = new Scanner(scanner.nextLine());

                number = lineReader.nextLong();


                if(number > 1)
                    output.append("\n");
               }

            System.out.println(output.toString());

            }
    }