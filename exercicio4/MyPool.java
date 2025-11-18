/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Criando um pool de threads em Java */

import java.util.LinkedList;
import java.util.Scanner;

//-------------------------------------------------------------------------------
//!!! Documentar essa classe !!!
class FilaTarefas {
    private final int nThreads;
    private final MyPoolThreads[] threads;
    private final LinkedList<Runnable> queue;
    private boolean shutdown;

    public FilaTarefas(int nThreads) {
        this.shutdown = false;
        this.nThreads = nThreads;
        queue = new LinkedList<Runnable>();
        threads = new MyPoolThreads[nThreads];
        for (int i=0; i<nThreads; i++) {
            threads[i] = new MyPoolThreads();
            threads[i].start();
        } 
    }

    public void execute(Runnable r) {
        synchronized(queue) {
            if (this.shutdown) return;
            queue.addLast(r);
            queue.notify();
        }
    }
    
    public void shutdown() {
        synchronized(queue) {
            this.shutdown=true;
            queue.notifyAll();
        }
        for (int i=0; i<nThreads; i++) {
          try { threads[i].join(); } catch (InterruptedException e) { return; }
        }
    }

    private class MyPoolThreads extends Thread {
       public void run() {
         Runnable r;
         while (true) {
           synchronized(queue) {
             while (queue.isEmpty() && (!shutdown)) {
               try { queue.wait(); }
               catch (InterruptedException ignored){}
             }
             if (queue.isEmpty()) return;   
             r = (Runnable) queue.removeFirst();
           }
           try { r.run(); }
           catch (RuntimeException e) {}
         } 
       } 
    } 
}
//-------------------------------------------------------------------------------

//--PASSO 1: cria uma classe que implementa a interface Runnable 
class Hello implements Runnable {
   String msg;
   public Hello(String m) { msg = m; }

   //--metodo executado pela thread
   public void run() {
      System.out.println(msg);
   }
}

class Primo implements Runnable {
   int num;
   
   public Primo(int n) { 
      num = n; 
   }

   public void run() {
      if (num < 2) {
         System.out.println(num + " não é um número primo");
         return;
      }
      
      for (int i = 2; i <= Math.sqrt(num); i++) {
         if (num % i == 0) {
            System.out.println(num + " não é um número primo");
            return;
         }
      }
      System.out.println(num + " é um número primo");
   }
}


class MyPool {
    public static void main(String[] args) {

        Scanner sc = new Scanner(System.in);

        System.out.print("Digite o número de threads: ");
        int nThreads = sc.nextInt();

        System.out.print("Digite o valor N (intervalo [1,N]): ");
        int N = sc.nextInt();

        FilaTarefas pool = new FilaTarefas(nThreads);

        for (int i = 1; i <= N; i++) {
            Runnable primo = new Primo(i);
            pool.execute(primo);
        }

        pool.shutdown();
        System.out.println("Terminou");

        sc.close();
    }
}